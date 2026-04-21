#ifndef MDF_LOGGER_HPP
#define MDF_LOGGER_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace logging {

// Captures CAN frames or decoded telemetry signals into an MDF4 (.mf4) file.
// One instance, one active file. Call stop() to finalize.
//
// CAN frames stream directly (MdfBusLogger preset). Telemetry signals buffer
// in RAM until stop(), because MDF4 requires all channels to be declared
// before InitMeasurement() — and telemetry channel names are only discovered
// as samples arrive.
class MdfLogger {
public:
  enum class Mode { None, CanRaw, TelemetrySignals };

  using LogCallback = std::function<void(const std::string &)>;

  MdfLogger();
  ~MdfLogger();

  MdfLogger(const MdfLogger &) = delete;
  MdfLogger &operator=(const MdfLogger &) = delete;

  void set_log_callback(LogCallback cb) { log_cb_ = std::move(cb); }
  void set_output_dir(const std::string &dir) { output_dir_ = dir; }

  // Begin a session. Creates output_dir_ if needed. Returns false on failure.
  bool start_can();
  bool start_telemetry();
  void stop();

  bool is_recording() const { return mode_ != Mode::None; }
  Mode mode() const { return mode_; }
  const std::string &current_path() const { return path_; }

  // CAN path (streaming).
  void log_can_frame(uint32_t id, uint8_t dlc,
                     const std::vector<uint8_t> &bytes);

  // Telemetry path (buffered). device_ts_ms is the device-side timestamp
  // carried in the JSON. The first sample's device ts anchors the timeline;
  // subsequent samples' times are relative to it.
  void log_signal_sample(const std::string &channel_name, double value,
                         uint32_t device_ts_ms);

private:
  struct TelemetrySample {
    std::string channel;
    uint32_t device_ts_ms;
    double value;
  };

  void emit_log(const std::string &msg);
  static std::string timestamped_filename();
  uint64_t now_ns() const;

  // Flush buffered telemetry samples out as an MDF4 file. Called by stop()
  // when in TelemetrySignals mode.
  bool flush_telemetry();

  Mode mode_ = Mode::None;
  std::string output_dir_ = "logs";
  std::string path_;
  LogCallback log_cb_;

  // CAN streaming state. Opaque pointers so callers don't pull in mdflib.
  struct CanState;
  std::unique_ptr<CanState> can_;
  uint64_t can_start_ns_ = 0;

  // Telemetry buffered state.
  std::vector<TelemetrySample> telemetry_buf_;
  uint64_t telemetry_start_ns_ = 0;
  uint32_t telemetry_first_device_ts_ms_ = 0;
  bool telemetry_have_first_ts_ = false;
};

} // namespace logging

#endif // MDF_LOGGER_HPP
