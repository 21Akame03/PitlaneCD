#ifndef DEBUG_VIEW_HPP
#define DEBUG_VIEW_HPP

#include "log_parser.hpp"

namespace DEBUG_LOG {

class DebugView {
public:
  DebugView();

  // Call every frame from the main render loop.
  // Polls new serial lines, parses, and renders both the log and plot windows.
  void RenderUI();

  // Clear all buffered data
  void Clear();

private:
  AppState state_;
  bool auto_scroll_;
  bool plot_follow_;
  bool plot_fit_requested_;
  float plot_history_secs_;
  char filter_buf_[256];

  void RenderLogWindow();
  void RenderPlotWindow();
};

} // namespace DEBUG_LOG

#endif // DEBUG_VIEW_HPP
