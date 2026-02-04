#ifndef CANDBC_PARSER_HPP
#define CANDBC_PARSER_HPP

#pragma once

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
};

class DBCParser {

public:
  std::string dbcfilepath_;

  // Constructors
  explicit DBCParser();

  // API
  bool load_dbc(const std::string &filepath);
  bool decode_message();

private:
  std::unique_ptr<Vector::DBC::Network> net_;
};

} // namespace CANDBC_PARSER

#endif
