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
  this->dbcfilepath_ = filepath;

  std::ifstream idbc(this->dbcfilepath_);
  auto net = std::make_unique<Vector::DBC::Network>();
  if (!idbc.is_open()) {
    std::cerr << "failed to open dbc file" << std::endl;
    return false;
  }

  idbc >> *net;

  if (!net->successfullyParsed) {
    std::cerr << "failed to parse dbc" << std::endl;
    return false;
  }

  this->net_ = std::move(net);

  return true;
};

/*
 * Purpose: Decode the CAN frame using the DBC file
 * Input: None
 * Output: signal value pair
 */
bool DBCParser::decode_message() {
  std::unordered_map<uint32_t, const Vector::DBC::Message *> messages;

  /* For every messages in the DBC file,
   * make a key data pair using the can_message id as key and the message as
   * value
   */

  for (const auto &msg_pair : this->net_->messages) {
    messages.insert(std::make_pair(msg_pair.first, &msg_pair.second));
  }

  can_frame_t frame;
  while (1) {
    // Receive CAN frame as byte from the ESP32 / CAN_sniffer and decode
  }

  return true;
};
} // namespace CANDBC_PARSER
