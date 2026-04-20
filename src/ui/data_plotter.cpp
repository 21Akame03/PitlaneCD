#include "ui/data_plotter.hpp"
#include "imgui.h"
#include "implot.h"

namespace ui {

DataPlotter::DataPlotter(
    int id,
    const std::unordered_map<std::string, debug::PlotSeries> &series)
    : id_(id), series_(&series) {}

bool DataPlotter::render_ui() {
  std::string title = "Plot #" + std::to_string(id_);
  if (!ImGui::Begin(title.c_str(), &open_)) {
    ImGui::End();
    return open_;
  }

  // --- Series checkboxes ---
  if (ImGui::TreeNode("Series")) {
    for (auto &[name, s] : *series_) {
      bool enabled = enabled_series_.count(name) > 0;
      if (ImGui::Checkbox(name.c_str(), &enabled)) {
        if (enabled)
          enabled_series_.insert(name);
        else
          enabled_series_.erase(name);
      }
    }
    ImGui::TreePop();
  }

  // --- Controls ---
  if (ImGui::Button("Fit"))
    fit_requested_ = true;
  ImGui::SameLine();
  ImGui::Checkbox("Follow", &follow_);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::SliderFloat("History (s)", &history_secs_, 1.0f, 120.0f, "%.0f s");

  // --- Find latest timestamp across enabled series ---
  double latest_ms = 0.0;
  for (auto &name : enabled_series_) {
    auto it = series_->find(name);
    if (it != series_->end() && !it->second.points.empty()) {
      double t = static_cast<double>(it->second.points.back().first);
      if (t > latest_ms)
        latest_ms = t;
    }
  }

  // --- Plot ---
  std::string plot_label = "##plot_" + std::to_string(id_);
  if (ImPlot::BeginPlot(plot_label.c_str(), ImVec2(-1, -1))) {
    ImPlot::SetupAxes("time (ms)", "value");

    if (fit_requested_) {
      ImPlot::SetupAxes("time (ms)", "value", ImPlotAxisFlags_AutoFit,
                        ImPlotAxisFlags_AutoFit);
      fit_requested_ = false;
    } else if (follow_ && latest_ms > 0.0) {
      double window_ms = static_cast<double>(history_secs_) * 1000.0;
      ImPlot::SetupAxisLimits(ImAxis_X1, latest_ms - window_ms, latest_ms,
                              ImPlotCond_Always);
    }

    for (auto &name : enabled_series_) {
      auto it = series_->find(name);
      if (it == series_->end() || it->second.points.empty())
        continue;

      auto &pts = it->second.points;
      thread_local std::vector<double> xs, ys;
      xs.resize(pts.size());
      ys.resize(pts.size());
      for (size_t i = 0; i < pts.size(); ++i) {
        xs[i] = static_cast<double>(pts[i].first);
        ys[i] = pts[i].second;
      }
      ImPlot::PlotLine(name.c_str(), xs.data(), ys.data(),
                       static_cast<int>(xs.size()));
    }

    ImPlot::EndPlot();
  }

  ImGui::End();
  return open_;
}

} // namespace ui
