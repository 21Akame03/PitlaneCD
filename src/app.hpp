#ifndef APP_HPP
#define APP_HPP

#include "can/dbc_parser.hpp"
#include "can/ig_panel.hpp"
#include "can/sniffer_panel.hpp"
#include "debug/debug_panel.hpp"
#include "serial/serial_reader.hpp"
#include "ui/app_log.hpp"
#include "ui/settings_panel.hpp"

struct App {
  ui::AppMode mode = ui::AppMode::None;

  serial::SerialReader serial_reader;
  can::DbcParser dbc_parser;
  AppLog log;

  can::SnifferPanel sniffer;
  can::IgPanel ig;
  debug::DebugPanel debug;
  ui::SettingsPanel settings;

  App();
  void RenderUI();

private:
  bool first_time_layout_ = true;
  void setup_default_docking_layout(unsigned int dockspace_id);
  void mode_selector();
};

#endif // APP_HPP
