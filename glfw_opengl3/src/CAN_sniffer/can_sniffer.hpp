#ifndef CAN_SNIFFER_HPP
#define CAN_SNIFFER_HPP

#include "CAN_sniffer/libs/candbc_parser.hpp"
#include <string>

namespace CAN_SNIFFER_WINDOW {

class Sniffer_window {
public:
  Sniffer_window();
  void RenderUI();
};

} // namespace CAN_SNIFFER_WINDOW

extern CANDBC_PARSER::DBCParser parser;

#endif
