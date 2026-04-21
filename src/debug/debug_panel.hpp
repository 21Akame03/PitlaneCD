#ifndef DEBUG_PANEL_HPP
#define DEBUG_PANEL_HPP

#include "debug/log_parser.hpp"
#include "logging/mdf_logger.hpp"
#include "serial/serial_reader.hpp"
#include "ui/data_plotter.hpp"
#include "ui/view_config.hpp"
#include <vector>

namespace debug {

class DebugPanel {
public:
  DebugPanel(serial::SerialReader &reader, logging::MdfLogger &mdf,
             const ui::AppMode &mode);

  void render_ui();
  void Clear();
  void add_plot();
  void add_plot(const ui::PlotConfig &cfg);
  std::vector<ui::PlotConfig> export_plot_configs() const;

private:
  AppState state_;
  serial::SerialReader &reader_;
  logging::MdfLogger &mdf_;
  const ui::AppMode &mode_;
  bool auto_scroll_;
  char filter_buf_[256];

  std::vector<ui::DataPlotter> plotters_;
  int next_plot_id_ = 1;

  void render_log_window();
};

} // namespace debug

#endif // DEBUG_PANEL_HPP
