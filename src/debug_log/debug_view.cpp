#include "debug_view.hpp"
#include "app_main.hpp"
#include "imgui.h"
#include "implot.h"

#include <algorithm>
#include <cstring>

namespace DEBUG_LOG {

DebugView::DebugView()
    : auto_scroll_(true),
      plot_follow_(true),
      plot_fit_requested_(false),
      plot_history_secs_(10.0f) {
  std::memset(filter_buf_, 0, sizeof(filter_buf_));
}

void DebugView::Clear() { state_.Clear(); }

// ──────────────────────────────────────────────
// Main entry — called each frame
// ──────────────────────────────────────────────

void DebugView::RenderUI() {
  // Poll new lines from serial reader
  auto lines = MyApp::serialReader.PollRxBuffer();
  for (auto &line : lines) {
    state_.Feed(line);
  }

  RenderLogWindow();
  RenderPlotWindow();
}

// ──────────────────────────────────────────────
// Live log view (coloured by level)
// ──────────────────────────────────────────────

void DebugView::RenderLogWindow() {
  ImGui::Begin("Log");

  // Toolbar
  if (ImGui::Button("Clear"))
    Clear();
  ImGui::SameLine();
  ImGui::Checkbox("Auto-scroll", &auto_scroll_);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(200);
  ImGui::InputText("Filter", filter_buf_, sizeof(filter_buf_));
  ImGui::Separator();

  // Scrollable region
  ImGui::BeginChild("LogScroll", ImVec2(0, 0), ImGuiChildFlags_None,
                     ImGuiWindowFlags_HorizontalScrollbar);

  bool has_filter = filter_buf_[0] != '\0';

  for (auto &e : state_.entries) {
    // Simple substring filter on tag + message
    if (has_filter) {
      bool match = e.tag.find(filter_buf_) != std::string::npos ||
                   e.message.find(filter_buf_) != std::string::npos;
      if (!match)
        continue;
    }

    float r, g, b, a;
    LevelColor(e.level, r, g, b, a);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r, g, b, a));

    // Format: [timestamp] [L] [tag] message
    ImGui::TextUnformatted(
        ("[" + std::to_string(e.timestamp_ms) + "] [" +
         std::string(1, "DIWE?"[static_cast<int>(e.level)]) + "] [" + e.tag +
         "] " + e.message)
            .c_str());

    ImGui::PopStyleColor();
  }

  if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    ImGui::SetScrollHereY(1.0f);

  ImGui::EndChild();
  ImGui::End();
}

// ──────────────────────────────────────────────
// Auto-discovered plot window
// ──────────────────────────────────────────────

void DebugView::RenderPlotWindow() {
  ImGui::Begin("Plotter Window");

  if (state_.series.empty()) {
    ImGui::TextDisabled("No plottable data yet. Send lines with | key=value");
    ImGui::End();
    return;
  }

  // ── Toolbar ──
  if (ImGui::Button("Fit"))
    plot_fit_requested_ = true;
  ImGui::SameLine();
  ImGui::Checkbox("Follow", &plot_follow_);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::SliderFloat("History (s)", &plot_history_secs_, 1.0f, 120.0f, "%.0f s");

  // Find the latest timestamp across all series (ms)
  double latest_ms = 0.0;
  for (auto &[name, s] : state_.series) {
    if (!s.points.empty()) {
      double t = static_cast<double>(s.points.back().first);
      if (t > latest_ms)
        latest_ms = t;
    }
  }

  if (ImPlot::BeginPlot("Debug Plot", ImVec2(-1, -1))) {
    ImPlot::SetupAxes("time (ms)", "value");

    if (plot_fit_requested_) {
      ImPlot::SetupAxes("time (ms)", "value",
                         ImPlotAxisFlags_AutoFit,
                         ImPlotAxisFlags_AutoFit);
      plot_fit_requested_ = false;
    } else if (plot_follow_ && latest_ms > 0.0) {
      double window_ms = static_cast<double>(plot_history_secs_) * 1000.0;
      ImPlot::SetupAxisLimits(ImAxis_X1, latest_ms - window_ms, latest_ms,
                               ImPlotCond_Always);
    }

    for (auto &[name, s] : state_.series) {
      if (s.points.empty())
        continue;

      thread_local std::vector<double> xs, ys;
      xs.resize(s.points.size());
      ys.resize(s.points.size());
      for (size_t i = 0; i < s.points.size(); ++i) {
        xs[i] = static_cast<double>(s.points[i].first);
        ys[i] = s.points[i].second;
      }
      ImPlot::PlotLine(name.c_str(), xs.data(), ys.data(),
                        static_cast<int>(xs.size()));
    }

    ImPlot::EndPlot();
  }

  ImGui::End();
}

} // namespace DEBUG_LOG
