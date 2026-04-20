#ifndef IG_PANEL_HPP
#define IG_PANEL_HPP

#include "can/dbc_parser.hpp"
#include "serial/serial_reader.hpp"
#include <cstdint>
#include <string>

namespace can {

class IgPanel {
public:
  IgPanel(DbcParser &parser, serial::SerialReader &reader);
  void render_ui();

private:
  void gen_tables();

  DbcParser &parser_;
  serial::SerialReader &reader_;

  bool hasSelection = false;
  uint32_t selectedMsgId = 0;
  std::string selectedSignalName;
  double inputValue = 0.0;
};

} // namespace can

#endif // IG_PANEL_HPP
