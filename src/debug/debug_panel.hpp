#ifndef DEBUG_PANEL_HPP
#define DEBUG_PANEL_HPP

#include "debug/log_parser.hpp"
#include "serial/serial_reader.hpp"

namespace debug {

class DebugPanel {
public:
  explicit DebugPanel(serial::SerialReader &reader);

  void render_ui();
  void Clear();

private:
  AppState state_;
  serial::SerialReader &reader_;
  bool auto_scroll_;
  bool plot_follow_;
  bool plot_fit_requested_;
  float plot_history_secs_;
  char filter_buf_[256];

  void render_log_window();
  void render_plot_window();
};

} // namespace debug

#endif // DEBUG_PANEL_HPP
