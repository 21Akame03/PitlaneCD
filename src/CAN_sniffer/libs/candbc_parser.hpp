#ifndef CANDBC_PARSER_HPP
#define CANDBC_PARSER_HPP

#include <vector>
#pragma once

#include "nlohmann/json.hpp"
#include <Vector/DBC/Network.h>
#include <cstdint>
#include <memory>
#include <string>

namespace CANDBC_PARSER {

// Typical CAN frame structure
struct can_frame_t {
  uint32_t id;
  uint8_t dlc;
  uint8_t __pad;
  uint8_t __res0;
  uint8_t __res1;
  uint8_t data[8];
  std::string name;
};

class DBCParser {

public:
  std::string dbcfilepath_;

  // Constructors
  explicit DBCParser();

  // API
  bool load_dbc(const std::string &filepath);
  bool is_loaded() const { return loaded_; }
  bool parse_frame();
  nlohmann::json parse_json();

private:
  std::vector<uint8_t> HexStringtoBytes(std::string hex);
  std::optional<Vector::DBC::Message> is_messagevalid(uint32_t id);
  std::unique_ptr<Vector::DBC::Network> net_;
  bool loaded_ = false;
};

} // namespace CANDBC_PARSER

#endif
