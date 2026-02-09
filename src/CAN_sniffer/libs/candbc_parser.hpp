#ifndef CANDBC_PARSER_HPP
#define CANDBC_PARSER_HPP

#include <optional>
#include <vector>
#pragma once

#include "nlohmann/json.hpp"
#include <Vector/DBC/Network.h>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace CANDBC_PARSER {

// Typical CAN frame structure
struct can_frame_t {
  uint32_t id;
  uint8_t dlc;
  double value;
  std::string name;
};

struct SignalValue {
    std::string name;
    double physicalValue;
    unsigned int rawValue;
    std::string unit;
    bool isMultiplexed;
};

struct MessageData {
    std::string name;
    uint32_t id;
    bool hasMultiplexedSignals = false;
    std::map<std::string, SignalValue> signals;
};

// Stores latest decoded values per message ID
extern std::map<uint32_t, MessageData> signal_store;

class DBCParser {

public:
  std::string dbcfilepath_;

  // Constructors
  explicit DBCParser();

  // API
  bool load_dbc(const std::string &filepath);
  bool is_loaded() const { return loaded_; }
  std::optional<can_frame_t> parse_frame(std::string data);
  nlohmann::json parse_json(std::string data);

private:
  std::vector<uint8_t> HexStringtoBytes(std::string hex);
  std::optional<Vector::DBC::Message> is_messagevalid(uint32_t id);
  std::unique_ptr<Vector::DBC::Network> net_;
  bool loaded_ = false;
};

} // namespace CANDBC_PARSER

#endif
