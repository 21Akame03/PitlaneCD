#include "logging/mdf_logger.hpp"

#include <mdf/canmessage.h>
#include <mdf/ichannel.h>
#include <mdf/ichannelgroup.h>
#include <mdf/idatagroup.h>
#include <mdf/ifilehistory.h>
#include <mdf/iheader.h>
#include <mdf/mdfenumerates.h>
#include <mdf/mdffactory.h>
#include <mdf/mdfwriter.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace logging {

struct MdfLogger::CanState {
  std::unique_ptr<mdf::MdfWriter> writer;
  mdf::IChannelGroup *data_frame_cg = nullptr; // owned by writer
};

MdfLogger::MdfLogger() = default;

MdfLogger::~MdfLogger() {
  if (is_recording())
    stop();
}

void MdfLogger::emit_log(const std::string &msg) {
  if (log_cb_) log_cb_(msg);
}

uint64_t MdfLogger::now_ns() const {
  return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());
}

std::string MdfLogger::timestamped_filename() {
  auto t = std::chrono::system_clock::to_time_t(
      std::chrono::system_clock::now());
  std::tm tm_local{};
#ifdef _WIN32
  localtime_s(&tm_local, &t);
#else
  localtime_r(&t, &tm_local);
#endif
  std::ostringstream ss;
  ss << "session_" << std::put_time(&tm_local, "%Y-%m-%d_%H-%M-%S") << ".mf4";
  return ss.str();
}

bool MdfLogger::start_can() {
  if (is_recording()) return false;

  std::error_code ec;
  std::filesystem::create_directories(output_dir_, ec);
  if (ec) {
    emit_log("MDF: failed to create " + output_dir_ + ": " + ec.message());
    return false;
  }

  path_ = (std::filesystem::path(output_dir_) / timestamped_filename()).string();

  try {
    can_ = std::make_unique<CanState>();
    can_->writer =
        mdf::MdfFactory::CreateMdfWriter(mdf::MdfWriterType::MdfBusLogger);
    if (!can_->writer || !can_->writer->Init(path_)) {
      emit_log("MDF: Init() failed for " + path_);
      can_.reset();
      path_.clear();
      return false;
    }

    auto *header = can_->writer->Header();
    if (header) {
      auto *history = header->CreateFileHistory();
      if (history) {
        history->Time(now_ns());
        history->ToolName("PitlaneCD");
        history->ToolVendor("PitlaneCD");
        history->ToolVersion("1.0");
        history->Description("CAN bus recording");
      }
    }

    can_->writer->BusType(mdf::MdfBusType::CAN);
    can_->writer->StorageType(mdf::MdfStorageType::MlsdStorage);
    can_->writer->MaxLength(8);
    can_->writer->CreateBusLogConfiguration();
    can_->writer->PreTrigTime(0.0);
    can_->writer->CompressData(false);

    auto *last_dg = header ? header->LastDataGroup() : nullptr;
    can_->data_frame_cg =
        last_dg ? last_dg->GetChannelGroup("CAN_DataFrame") : nullptr;
    if (!can_->data_frame_cg) {
      emit_log("MDF: CAN_DataFrame channel group not found");
      can_.reset();
      path_.clear();
      return false;
    }

    can_start_ns_ = now_ns();
    can_->writer->InitMeasurement();
    can_->writer->StartMeasurement(can_start_ns_);
  } catch (const std::exception &e) {
    emit_log(std::string("MDF: start_can failed: ") + e.what());
    can_.reset();
    path_.clear();
    return false;
  }

  mode_ = Mode::CanRaw;
  emit_log("MDF: recording CAN frames to " + path_);
  return true;
}

bool MdfLogger::start_telemetry() {
  if (is_recording()) return false;

  std::error_code ec;
  std::filesystem::create_directories(output_dir_, ec);
  if (ec) {
    emit_log("MDF: failed to create " + output_dir_ + ": " + ec.message());
    return false;
  }

  path_ = (std::filesystem::path(output_dir_) / timestamped_filename()).string();

  telemetry_buf_.clear();
  telemetry_start_ns_ = now_ns();
  telemetry_have_first_ts_ = false;
  telemetry_first_device_ts_ms_ = 0;

  mode_ = Mode::TelemetrySignals;
  emit_log("MDF: buffering telemetry signals for " + path_);
  return true;
}

void MdfLogger::stop() {
  if (!is_recording()) return;

  if (mode_ == Mode::CanRaw) {
    try {
      if (can_ && can_->writer) {
        can_->writer->StopMeasurement(now_ns());
        can_->writer->FinalizeMeasurement();
      }
      emit_log("MDF: CAN recording stopped: " + path_);
    } catch (const std::exception &e) {
      emit_log(std::string("MDF: finalize failed: ") + e.what());
    }
    can_.reset();
  } else if (mode_ == Mode::TelemetrySignals) {
    flush_telemetry();
  }

  mode_ = Mode::None;
  path_.clear();
}

void MdfLogger::log_can_frame(uint32_t id, uint8_t dlc,
                              const std::vector<uint8_t> &bytes) {
  if (mode_ != Mode::CanRaw || !can_ || !can_->writer || !can_->data_frame_cg)
    return;

  try {
    mdf::CanMessage msg;
    msg.MessageId(id);
    msg.ExtendedId(id > 0x7FF);
    msg.Dlc(dlc);
    msg.BusChannel(0);
    if (!bytes.empty())
      msg.DataBytes(bytes);

    can_->writer->SaveCanMessage(*can_->data_frame_cg, now_ns(), msg);
  } catch (const std::exception &e) {
    emit_log(std::string("MDF: SaveCanMessage failed: ") + e.what());
  }
}

void MdfLogger::log_signal_sample(const std::string &channel_name, double value,
                                  uint32_t device_ts_ms) {
  if (mode_ != Mode::TelemetrySignals) return;

  if (!telemetry_have_first_ts_) {
    telemetry_first_device_ts_ms_ = device_ts_ms;
    telemetry_have_first_ts_ = true;
  }
  telemetry_buf_.push_back({channel_name, device_ts_ms, value});
}

// Writes the buffered telemetry samples to an MDF4 file.
// One DataGroup, one ChannelGroup per unique channel name. Each CG has a
// Time master (seconds, FloatLe) and a value channel (FloatLe, double).
bool MdfLogger::flush_telemetry() {
  if (telemetry_buf_.empty()) {
    emit_log("MDF: telemetry session had no samples, skipping file: " + path_);
    return true;
  }

  try {
    auto writer =
        mdf::MdfFactory::CreateMdfWriter(mdf::MdfWriterType::Mdf4Basic);
    if (!writer || !writer->Init(path_)) {
      emit_log("MDF: telemetry Init() failed for " + path_);
      telemetry_buf_.clear();
      return false;
    }

    auto *header = writer->Header();
    if (header) {
      header->Author("PitlaneCD");
      header->Description("Telemetry signal recording");
      header->StartTime(telemetry_start_ns_);
      auto *history = header->CreateFileHistory();
      if (history) {
        history->Time(telemetry_start_ns_);
        history->ToolName("PitlaneCD");
        history->ToolVendor("PitlaneCD");
        history->ToolVersion("1.0");
        history->Description("Telemetry");
      }
    }

    // Collect unique channel names in first-seen order.
    std::vector<std::string> names;
    std::unordered_set<std::string> seen;
    for (const auto &s : telemetry_buf_) {
      if (seen.insert(s.channel).second) names.push_back(s.channel);
    }

    auto *dg = writer->CreateDataGroup();
    if (!dg) {
      emit_log("MDF: telemetry CreateDataGroup failed");
      telemetry_buf_.clear();
      return false;
    }

    struct GroupEntry {
      mdf::IChannelGroup *cg = nullptr;
      mdf::IChannel *time = nullptr;
      mdf::IChannel *value = nullptr;
    };
    std::unordered_map<std::string, GroupEntry> groups;

    for (const auto &name : names) {
      auto *cg = dg->CreateChannelGroup();
      if (!cg) continue;
      cg->Name(name);

      auto *time_ch = cg->CreateChannel();
      if (time_ch) {
        time_ch->Name("t");
        time_ch->Type(mdf::ChannelType::Master);
        time_ch->Sync(mdf::ChannelSyncType::Time);
        time_ch->DataType(mdf::ChannelDataType::FloatLe);
        time_ch->DataBytes(sizeof(double));
        time_ch->Unit("s");
      }

      auto *val_ch = cg->CreateChannel();
      if (val_ch) {
        val_ch->Name(name);
        val_ch->Type(mdf::ChannelType::FixedLength);
        val_ch->DataType(mdf::ChannelDataType::FloatLe);
        val_ch->DataBytes(sizeof(double));
      }

      groups[name] = {cg, time_ch, val_ch};
    }

    writer->InitMeasurement();
    writer->StartMeasurement(telemetry_start_ns_);

    // Samples are written in arrival order. Absolute sample time is
    // telemetry_start_ns_ + (device_ts_ms - first_device_ts_ms) * 1e6 ns.
    for (const auto &s : telemetry_buf_) {
      auto it = groups.find(s.channel);
      if (it == groups.end() || !it->second.cg || !it->second.value) continue;
      const uint32_t rel_ms = s.device_ts_ms - telemetry_first_device_ts_ms_;
      const uint64_t abs_ns =
          telemetry_start_ns_ + static_cast<uint64_t>(rel_ms) * 1'000'000ull;
      it->second.value->SetChannelValue(s.value);
      writer->SaveSample(*it->second.cg, abs_ns);
    }

    writer->StopMeasurement(now_ns());
    writer->FinalizeMeasurement();

    emit_log("MDF: telemetry recording stopped: " + path_ + " (" +
             std::to_string(telemetry_buf_.size()) + " samples, " +
             std::to_string(names.size()) + " channels)");
  } catch (const std::exception &e) {
    emit_log(std::string("MDF: telemetry flush failed: ") + e.what());
    telemetry_buf_.clear();
    return false;
  }

  telemetry_buf_.clear();
  return true;
}

} // namespace logging
