#include "candbc_parser.hpp"
#include "CAN_sniffer/can_sniffer.hpp"
#include "app_main.hpp"

#include "nlohmann/json.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>

#include <Vector/DBC.h>
#include <vector>

std::map<uint32_t, CANDBC_PARSER::can_frame_t> can_frame_list;

bool addtocanframelist(uint32_t id, std::string name, double physicalValue) {
  // check if the signal is already in the list
  if (can_frame_list.find(id) != can_frame_list.end()) {
  }
}

namespace CANDBC_PARSER {

// Constructor
DBCParser::DBCParser() {};

/*
 * Purpose: Get the network from the DBC file and check if it is presents
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

/*
 * Purpose: Parse the JSON frame and return the result
 * Input: NONE
 * Output: JSON frame
 */
nlohmann::json DBCParser::parse_json(std::string data) {

  // parse the json received, return discarded on error instead of throwing
  auto j = nlohmann::json::parse(data, nullptr, false);
  return j;
}

/*
 * Purpose: Check if the message id is present in the DBC file
 *
 */
std::optional<Vector::DBC::Message> DBCParser::is_messagevalid(uint32_t id) {
  // find the message with the id in the network
  for (const auto &message : this->net_->messages) {
    if (message.second.id == id) {
      return message.second;
    }
  }

  return std::nullopt;
}

/*
 * Purpose: Convert a Hex string to a vector of bytes
 * Input: Hex string
 * Output: Vextor of Bytes
 */
std::vector<uint8_t> DBCParser::HexStringtoBytes(std::string hex) {
  std::vector<uint8_t> bytes;

  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    bytes.push_back(
        static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16)));
  }

  return bytes;
}

/*
 * Purpose: Parse the CAN frame and return the result
 *
 */
std::optional<can_frame_t> DBCParser::parse_frame(std::string data) {

  // start finding out the value from the dbc file
  // lets start parsing
  nlohmann::json json_fram = parse_json(data);

  // skip non-JSON or malformed lines
  if (json_fram.is_discarded()) {
    return std::nullopt;
  }

  // bail out if no DBC is loaded – net_ would be null
  if (!loaded_) {
    return std::nullopt;
  }

  // now we need the id to identify the message
  uint32_t id = json_fram["id"];
  std::string hex_data = json_fram["data"];
  std::vector<uint8_t> can_data = HexStringtoBytes(hex_data);

  // we check if there is a message with the id in the dbc file
  auto message = is_messagevalid(id);
  // if message is not found
  if (!message) {
    return std::nullopt;
  }
  CAN_MODE_WINDOW::log.AddLog("Message: %s\n", message->name.c_str());

  // loop over the message to find multiplexor
  unsigned int multiplexerSwitchValue = 0;
  for (const auto &signal : message->signals) {
    if (signal.second.multiplexor ==
        Vector::DBC::Signal::Multiplexor::MultiplexorSwitch) {
      unsigned int rawvalue = signal.second.decode(can_data);
      multiplexerSwitchValue = rawvalue;
      CAN_MODE_WINDOW::log.AddLog("  Multiplexed message switch value = %u\n",
                                  multiplexerSwitchValue);
    }
  }

  /* loop over signals of this messages */
  for (const auto &signal : message->signals) {
    switch (signal.second.multiplexor) {
    case Vector::DBC::Signal::Multiplexor::MultiplexorSwitch: {
      unsigned int rawValue = signal.second.decode(can_data);
      CAN_MODE_WINDOW::log.AddLog("  Signal (MultiplexorSwitch) %s  Raw: 0x%X\n",
                                  signal.second.name.c_str(), rawValue);
    } break;
    case Vector::DBC::Signal::Multiplexor::MultiplexedSignal: {
      unsigned int rawValue = signal.second.decode(can_data);
      double physicalValue = signal.second.rawToPhysicalValue(rawValue);
      CAN_MODE_WINDOW::log.AddLog("  Signal (MultiplexedSignal) %s  Raw: 0x%X  Physical: %.2f\n",
                                  signal.second.name.c_str(), rawValue, physicalValue);
    } break;
    case Vector::DBC::Signal::Multiplexor::NoMultiplexor: {
      unsigned int rawValue = signal.second.decode(can_data);
      double physicalValue = signal.second.rawToPhysicalValue(rawValue);
      CAN_MODE_WINDOW::log.AddLog("  Signal %s  Raw: 0x%X  Physical: %.2f\n",
                                  signal.second.name.c_str(), rawValue, physicalValue);
    } break;
    }
  }

  return std::nullopt;
}

//
// void decodeMessage(unsigned int &canIdentifier,
//                    std::vector<std::uint8_t> &canData) {
//   /* get the relevant message from the database */
//   Vector::DBC::Message &message = network.messages[canIdentifier];
//   std::cout << "Message " << message.name << std::endl;
//
//   /* loop over signals of this message to find and get multiplexor */
//   unsigned int multiplexerSwitchValue = 0;
//   for (const auto &signal : message.signals) {
//     if (signal.second.multiplexor ==
//         Vector::DBC::Signal::Multiplexor::MultiplexorSwitch) {
//       unsigned int rawValue = signal.second.decode(canData);
//       multiplexerSwitchValue = rawValue;
//       std::cout << "  this is a multiplexed message with switch value = "
//                 << std::dec << multiplexerSwitchValue << std::endl;
//     }
//   }
//
//   /* loop over signals of this messages */
//   for (const auto &signal : message.signals) {
//     switch (signal.second.multiplexor) {
//     case Vector::DBC::Signal::Multiplexor::MultiplexorSwitch: {
//       /* if it's the multiplexorSwitch, only show raw value */
//       std::cout << "  Signal (MultiplexorSwitch) " << signal.second.name
//                 << std::endl;
//       unsigned int rawValue = signal.second.decode(canData);
//       std::cout << "    Raw Value: 0x" << std::hex << rawValue << std::endl;
//     } break;
//     case Vector::DBC::Signal::Multiplexor::MultiplexedSignal: {
//       /* if it's an multiplexed signal check that the value matches */
//       std::cout << "  Signal (MultiplexedSignal) " << signal.second.name
//                 << std::endl;
//       unsigned int rawValue = signal.second.decode(canData);
//       std::cout << "    Raw Value: 0x" << std::hex << rawValue << std::endl;
//       double physicalValue = signal.second.rawToPhysicalValue(rawValue);
//       std::cout << "    Physical Value: " << physicalValue << std::endl;
//     } break;
//     case Vector::DBC::Signal::Multiplexor::NoMultiplexor: {
//       /* if it's a not a multiplexed signal just proceed it */
//       std::cout << "  Signal " << signal.second.name << std::endl;
//       unsigned int rawValue = signal.second.decode(canData);
//       std::cout << "    Raw Value: 0x" << std::hex << rawValue << std::endl;
//       double physicalValue = signal.second.rawToPhysicalValue(rawValue);
//       std::cout << "    Physical Value: " << physicalValue << std::endl;
//     } break;
//     }
//   }
// }

} // namespace CANDBC_PARSER
