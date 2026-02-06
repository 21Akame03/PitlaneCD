#ifndef CAN_SNIFFER_HPP
#define CAN_SNIFFER_HPP

#include "CAN_sniffer/libs/candbc_parser.hpp"
#include <string>

namespace CAN_MODE_WINDOW {

class Sniffer_window {
public:
  Sniffer_window();
  void RenderUI();
};

extern CANDBC_PARSER::DBCParser parser;

} // namespace CAN_MODE_WINDOW

namespace CAN_IG {

class CAN_IG {
public:
  CAN_IG();
  void RenderUI();

private:
  static void gen_tables();
};
} // namespace CAN_IG
#endif
