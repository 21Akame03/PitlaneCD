#ifndef SETTINGS_PANEL_HPP
#define SETTINGS_PANEL_HPP

#include "can/dbc_parser.hpp"
#include "logging/mdf_logger.hpp"
#include "serial/serial_reader.hpp"
#include <functional>

namespace ui {

enum class AppMode { None, CanSniffer, Debug, Telemetry };

class SettingsPanel {
public:
  SettingsPanel(serial::SerialReader &reader, can::DbcParser &parser,
                AppMode &mode, logging::MdfLogger &mdf);
  void render_ui();

  void set_on_new_plot(std::function<void()> cb) {
    on_new_plot_ = std::move(cb);
  }
  void set_on_save_view(std::function<void(const std::string &)> cb) {
    on_save_view_ = std::move(cb);
  }

  const std::string &dbc_path() const { return dbc_file_path_; }
  void set_dbc_path(const std::string &p) { dbc_file_path_ = p; }
  void flash_saved() { saved_feedback_timer_ = 1.5f; }

private:
  void connection_selector();
  void dbc_selector();
  void recording_controls();
  void auto_start_logging();

  serial::SerialReader &reader_;
  can::DbcParser &parser_;
  AppMode &mode_;
  logging::MdfLogger &mdf_;
  std::function<void()> on_new_plot_;
  std::function<void(const std::string &)> on_save_view_;
  float saved_feedback_timer_ = 0.0f;

  std::vector<std::string> com_ports_{"Disconnected"};
  int current_port_ = 0;
  bool is_connected_ = false;
  bool was_connected_ = false;
  std::string dbc_file_path_;
};

} // namespace ui

#endif // SETTINGS_PANEL_HPP
