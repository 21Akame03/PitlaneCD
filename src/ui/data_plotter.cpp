#include "ui/data_plotter.hpp"
#include "imgui.h"
#include "implot.h"

namespace ui {

DataPlotter::DataPlotter(
    int id,
    const std::unordered_map<std::string, debug::PlotSeries> &series)
    : id_(id), series_(&series) {}

DataPlotter::DataPlotter(
    int id,
    const std::unordered_map<std::string, debug::PlotSeries> &series,
    const PlotConfig &cfg)
    : id_(id), series_(&series), enabled_series_(cfg.enabled_series),
      follow_(cfg.follow), auto_fit_(cfg.auto_fit), show_max_(cfg.show_max),
      show_min_(cfg.show_min), history_secs_(cfg.history_secs) {}

PlotConfig DataPlotter::export_config() const {
  PlotConfig c;
  c.enabled_series = enabled_series_;
  c.history_secs = history_secs_;
  c.follow = follow_;
  c.auto_fit = auto_fit_;
  c.show_max = show_max_;
  c.show_min = show_min_;
  return c;
}

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
  ImGui::Checkbox("Auto-Fit", &auto_fit_);
  ImGui::SameLine();
  ImGui::Checkbox("Max", &show_max_);
  ImGui::SameLine();
  ImGui::Checkbox("Min", &show_min_);
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
    } else {
      ImPlot::SetupAxes("time (ms)", "value", 0,
                        auto_fit_ ? ImPlotAxisFlags_AutoFit : 0);
    }
    if (!fit_requested_ && follow_ && latest_ms > 0.0) {
      double window_ms = static_cast<double>(history_secs_) * 1000.0;
      ImPlot::SetupAxisLimits(ImAxis_X1, latest_ms - window_ms, latest_ms,
                              ImPlotCond_Always);
    }

    struct SeriesInfo {
      std::string name;
      double mn, mx;
      ImVec4 color;
      const std::vector<std::pair<uint32_t, double>> *pts;
    };
    std::vector<SeriesInfo> infos;

    for (auto &name : enabled_series_) {
      auto it = series_->find(name);
      if (it == series_->end() || it->second.points.empty())
        continue;

      auto &pts = it->second.points;
      thread_local std::vector<double> xs, ys;
      xs.resize(pts.size());
      ys.resize(pts.size());
      double mn = pts[0].second, mx = pts[0].second;
      for (size_t i = 0; i < pts.size(); ++i) {
        xs[i] = static_cast<double>(pts[i].first);
        ys[i] = pts[i].second;
        if (ys[i] < mn)
          mn = ys[i];
        if (ys[i] > mx)
          mx = ys[i];
      }
      ImPlot::PlotLine(name.c_str(), xs.data(), ys.data(),
                       static_cast<int>(xs.size()));
      ImVec4 col = ImPlot::GetLastItemColor();
      infos.push_back({name, mn, mx, col, &pts});
    }

    if (show_max_ || show_min_) {
      ImDrawList *dl = ImPlot::GetPlotDrawList();
      ImPlotRect lim = ImPlot::GetPlotLimits();
      auto draw_dashed = [&](double y, ImU32 col) {
        ImVec2 p1 = ImPlot::PlotToPixels(ImPlotPoint(lim.X.Min, y));
        ImVec2 p2 = ImPlot::PlotToPixels(ImPlotPoint(lim.X.Max, y));
        const float dash = 8.0f, gap = 6.0f;
        float x = p1.x;
        while (x < p2.x) {
          float x2 = x + dash;
          if (x2 > p2.x)
            x2 = p2.x;
          dl->AddLine(ImVec2(x, p1.y), ImVec2(x2, p1.y), col, 1.5f);
          x += dash + gap;
        }
      };
      ImPlot::PushPlotClipRect();
      for (auto &si : infos) {
        ImU32 col = ImGui::ColorConvertFloat4ToU32(si.color);
        ImVec2 p_left =
            ImPlot::PlotToPixels(ImPlotPoint(lim.X.Min, 0.0));
        if (show_max_) {
          draw_dashed(si.mx, col);
          char buf[64];
          snprintf(buf, sizeof(buf), "%s max: %.3f", si.name.c_str(), si.mx);
          ImVec2 pp = ImPlot::PlotToPixels(ImPlotPoint(lim.X.Min, si.mx));
          dl->AddText(ImVec2(p_left.x + 4.0f, pp.y - 14.0f), col, buf);
        }
        if (show_min_) {
          draw_dashed(si.mn, col);
          char buf[64];
          snprintf(buf, sizeof(buf), "%s min: %.3f", si.name.c_str(), si.mn);
          ImVec2 pp = ImPlot::PlotToPixels(ImPlotPoint(lim.X.Min, si.mn));
          dl->AddText(ImVec2(p_left.x + 4.0f, pp.y + 2.0f), col, buf);
        }
      }
      ImPlot::PopPlotClipRect();
    }

    if (ImPlot::IsPlotHovered() && !infos.empty()) {
      ImPlotPoint mp = ImPlot::GetPlotMousePos();
      ImDrawList *dl = ImPlot::GetPlotDrawList();
      ImPlot::PushPlotClipRect();
      ImVec2 top = ImPlot::PlotToPixels(
          ImPlotPoint(mp.x, ImPlot::GetPlotLimits().Y.Max));
      ImVec2 bot = ImPlot::PlotToPixels(
          ImPlotPoint(mp.x, ImPlot::GetPlotLimits().Y.Min));
      dl->AddLine(top, bot, IM_COL32(150, 150, 150, 120), 1.0f);
      ImPlot::PopPlotClipRect();

      ImGui::BeginTooltip();
      ImGui::Text("t: %.0f ms", mp.x);
      for (auto &si : infos) {
        auto &pts = *si.pts;
        size_t lo = 0, hi = pts.size();
        while (lo < hi) {
          size_t m = (lo + hi) / 2;
          if (static_cast<double>(pts[m].first) < mp.x)
            lo = m + 1;
          else
            hi = m;
        }
        size_t idx = lo;
        if (idx >= pts.size())
          idx = pts.size() - 1;
        if (idx > 0) {
          double d1 = std::abs(static_cast<double>(pts[idx].first) - mp.x);
          double d0 = std::abs(static_cast<double>(pts[idx - 1].first) - mp.x);
          if (d0 < d1)
            idx = idx - 1;
        }
        ImGui::TextColored(si.color, "%s: %.3f", si.name.c_str(),
                           pts[idx].second);
      }
      ImGui::EndTooltip();
    }

    ImPlot::EndPlot();
  }

  ImGui::End();
  return open_;
}

} // namespace ui
