#include "can/dbc_parser.hpp"

#include "nlohmann/json.hpp"
#include <Vector/DBC.h>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>

namespace can {

DbcParser::DbcParser() {}

void DbcParser::log(const char *fmt, ...) {
  if (!log_cb_)
    return;
  char buf[512];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  log_cb_(std::string(buf));
}

// Skip reload if the same file is already active
bool DbcParser::request_load(const std::string &path) {
  if (path.empty())
    return false;
  if (loaded_ && dbcfilepath_ == path)
    return true;
  return load_dbc(path);
}

bool DbcParser::load_dbc(const std::string &filepath) {
  if (loaded_ && dbcfilepath_ == filepath)
    return true;

  dbcfilepath_ = filepath;

  std::ifstream idbc(dbcfilepath_);
  auto net = std::make_unique<Vector::DBC::Network>();
  if (!idbc.is_open()) {
    std::cerr << "failed to open dbc file" << std::endl;
    loaded_ = false;
    return false;
  }

  idbc >> *net;

  if (!net->successfullyParsed) {
    std::cerr << "failed to parse dbc" << std::endl;
    loaded_ = false;
    return false;
  }

  net_ = std::move(net);
  loaded_ = true;
  return true;
}

nlohmann::json DbcParser::parse_json(const std::string &data) {
  return nlohmann::json::parse(data, nullptr, false);
}

std::optional<Vector::DBC::Message> DbcParser::find_message(uint32_t id) {
  for (const auto &message : net_->messages) {
    if (message.second.id == id)
      return message.second;
  }
  return std::nullopt;
}

std::vector<uint8_t> DbcParser::hex_to_bytes(const std::string &hex) {
  std::vector<uint8_t> bytes;
  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    bytes.push_back(
        static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16)));
  }
  return bytes;
}

std::optional<CanFrame> DbcParser::parse_frame(const std::string &data) {
  nlohmann::json json_frame = parse_json(data);

  if (json_frame.is_discarded())
    return std::nullopt;
  if (!json_frame.contains("id") || !json_frame.contains("data"))
    return std::nullopt;

  uint32_t id = json_frame["id"];
  std::string hex_data = json_frame["data"];
  std::vector<uint8_t> can_data = hex_to_bytes(hex_data);

  CanFrame frame;
  frame.id = id;
  frame.dlc = static_cast<uint8_t>(can_data.size());
  frame.data = can_data;

  if (!loaded_)
    return frame;

  auto message = find_message(id);
  if (!message)
    return frame;

  frame.name = message->name;
  log("Message: %s\n", message->name.c_str());

  auto &msgData = signal_store_[id];
  msgData.name = message->name;
  msgData.id = id;

  // First pass: extract the mux switch value so we know which multiplexed signals are active
  unsigned int multiplexerSwitchValue = 0;
  for (const auto &signal : message->signals) {
    if (signal.second.multiplexor ==
        Vector::DBC::Signal::Multiplexor::MultiplexorSwitch) {
      unsigned int rawvalue = signal.second.decode(can_data);
      multiplexerSwitchValue = rawvalue;
      log("  Multiplexed message switch value = %u\n",
          multiplexerSwitchValue);
    }
  }

  // Second pass: decode all signals and update the live signal store
  for (const auto &signal : message->signals) {
    switch (signal.second.multiplexor) {
    case Vector::DBC::Signal::Multiplexor::MultiplexorSwitch: {
      unsigned int rawValue = signal.second.decode(can_data);
      log("  Signal (MultiplexorSwitch) %s  Raw: 0x%X\n",
          signal.second.name.c_str(), rawValue);
      msgData.hasMultiplexedSignals = true;
      msgData.signals[signal.second.name] = {signal.second.name,
                                             static_cast<double>(rawValue),
                                             rawValue, signal.second.unit,
                                             true};
    } break;
    case Vector::DBC::Signal::Multiplexor::MultiplexedSignal: {
      unsigned int rawValue = signal.second.decode(can_data);
      double physicalValue = signal.second.rawToPhysicalValue(rawValue);
      log("  Signal (MultiplexedSignal) %s  Raw: 0x%X  Physical: %.2f\n",
          signal.second.name.c_str(), rawValue, physicalValue);
      msgData.hasMultiplexedSignals = true;
      msgData.signals[signal.second.name] = {
          signal.second.name, physicalValue, rawValue, signal.second.unit,
          true};
    } break;
    case Vector::DBC::Signal::Multiplexor::NoMultiplexor: {
      unsigned int rawValue = signal.second.decode(can_data);
      double physicalValue = signal.second.rawToPhysicalValue(rawValue);
      log("  Signal %s  Raw: 0x%X  Physical: %.2f\n",
          signal.second.name.c_str(), rawValue, physicalValue);
      msgData.signals[signal.second.name] = {
          signal.second.name, physicalValue, rawValue, signal.second.unit,
          false};
    } break;
    }
  }

  return frame;
}

} // namespace can
