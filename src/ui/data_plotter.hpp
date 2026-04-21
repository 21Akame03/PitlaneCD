#ifndef DATA_PLOTTER_PANEL
#define DATA_PLOTTER_PANEL

#include "debug/log_parser.hpp"
#include "ui/view_config.hpp"
#include <set>
#include <string>
#include <unordered_map>

namespace ui {

class DataPlotter {
public:
  // Each instance gets a unique window title (e.g. "Plot #1", "Plot #2")
  explicit DataPlotter(
      int id,
      const std::unordered_map<std::string, debug::PlotSeries> &series);

  DataPlotter(int id,
              const std::unordered_map<std::string, debug::PlotSeries> &series,
              const PlotConfig &cfg);

  // Returns false if the user closed this plot window
  bool render_ui();

  int id() const { return id_; }
  PlotConfig export_config() const;

private:
  int id_;
  const std::unordered_map<std::string, debug::PlotSeries> *series_;
  std::set<std::string> enabled_series_;
  bool open_ = true;
  bool follow_ = true;
  bool fit_requested_ = false;
  bool auto_fit_ = false;
  bool show_max_ = false;
  bool show_min_ = false;
  float history_secs_ = 10.0f;
};

} // namespace ui

#endif
