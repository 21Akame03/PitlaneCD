#ifndef APP_HPP
#define APP_HPP

#include "can/dbc_parser.hpp"
#include "can/ig_panel.hpp"
#include "can/sniffer_panel.hpp"
#include "debug/debug_panel.hpp"
#include "serial/serial_reader.hpp"
#include "ui/app_log.hpp"
#include "ui/settings_panel.hpp"
#include "ui/view_config.hpp"

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
  bool view_prompt_pending_ = false;
  bool view_browsing_ = false;
  void setup_default_docking_layout(unsigned int dockspace_id);
  void mode_selector();
  void view_prompt();
  void apply_view_config();
  void save_current_view(const std::string &path);
  void load_view_from(const std::string &path);

  ui::ViewConfig pending_view_;
  std::string last_view_path_;
};

#endif // APP_HPP
