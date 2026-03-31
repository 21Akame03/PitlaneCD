#ifndef SETTINGS_PANEL_HPP
#define SETTINGS_PANEL_HPP

#include "can/dbc_parser.hpp"
#include "serial/serial_reader.hpp"

namespace ui {

enum class AppMode { None, CanSniffer, Debug };

class SettingsPanel {
public:
  SettingsPanel(serial::SerialReader &reader, can::DbcParser &parser,
                AppMode &mode);
  void render_ui();

private:
  void connection_selector();
  void dbc_selector();

  serial::SerialReader &reader_;
  can::DbcParser &parser_;
  AppMode &mode_;

  std::vector<std::string> com_ports_{"Disconnected"};
  int current_port_ = 0;
  bool is_connected_ = false;
  std::string dbc_file_path_;
};

} // namespace ui

#endif // SETTINGS_PANEL_HPP
