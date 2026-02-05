#include "candbc_parser.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

#include <Vector/DBC.h>

namespace CANDBC_PARSER {

// Constructor
DBCParser::DBCParser() {};

/*
 * Purpose: Get the network from the DBC file and check if it is present
 * Input: DBC filepath
 * Output: true if successful
 */
bool DBCParser::load_dbc(const std::string &filepath) {
  // Skip if already loaded with the same file
  if (loaded_ && dbcfilepath_ == filepath) {
    return true;
  }

  this->dbcfilepath_ = filepath;

  std::ifstream idbc(this->dbcfilepath_);
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

  this->net_ = std::move(net);
  loaded_ = true;

  return true;
}
} // namespace CANDBC_PARSER
