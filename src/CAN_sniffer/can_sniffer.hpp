#ifndef CAN_SNIFFER_HPP
#define CAN_SNIFFER_HPP

#include "CAN_sniffer/libs/candbc_parser.hpp"
#include "components/components.hpp"
#include "imgui.h"
#include <cstdarg>
#include <string>


namespace CAN_MODE_WINDOW {

extern AppLog log;

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

  // Currently selected signal (set by user clicking in the IG table)
  bool hasSelection = false;
  uint32_t selectedMsgId = 0;
  std::string selectedSignalName;

  // Physical value entered by user for sending
  double inputValue = 0.0;

private:
  void gen_tables();
};
} // namespace CAN_IG
#endif
