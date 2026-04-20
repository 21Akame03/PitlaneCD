#ifndef DEBUG_PANEL_HPP
#define DEBUG_PANEL_HPP

#include "debug/log_parser.hpp"
#include "serial/serial_reader.hpp"
#include "ui/data_plotter.hpp"
#include <vector>

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
  char filter_buf_[256];

  std::vector<ui::DataPlotter> plotters_;
  int next_plot_id_ = 1;

  void render_log_window();
};

} // namespace debug

#endif // DEBUG_PANEL_HPP
