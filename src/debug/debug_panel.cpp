#include "debug/debug_panel.hpp"
#include "imgui.h"

#include <algorithm>
#include <cstring>

namespace debug {

DebugPanel::DebugPanel(serial::SerialReader &reader)
    : reader_(reader), auto_scroll_(true) {
  std::memset(filter_buf_, 0, sizeof(filter_buf_));
}

void DebugPanel::Clear() { state_.Clear(); }

void DebugPanel::add_plot() {
  plotters_.emplace_back(next_plot_id_++, state_.series);
}

void DebugPanel::add_plot(const ui::PlotConfig &cfg) {
  plotters_.emplace_back(next_plot_id_++, state_.series, cfg);
}

std::vector<ui::PlotConfig> DebugPanel::export_plot_configs() const {
  std::vector<ui::PlotConfig> out;
  out.reserve(plotters_.size());
  for (auto &p : plotters_) out.push_back(p.export_config());
  return out;
}

void DebugPanel::render_ui() {
  auto lines = reader_.PollRxBuffer();
  for (auto &line : lines) {
    state_.Feed(line);
  }

  render_log_window();

  // Render all open plotters, remove closed ones
  plotters_.erase(
      std::remove_if(plotters_.begin(), plotters_.end(),
                     [](ui::DataPlotter &p) { return !p.render_ui(); }),
      plotters_.end());
}

void DebugPanel::render_log_window() {
  ImGui::Begin("Log");

  if (ImGui::Button("Clear"))
    Clear();
  ImGui::SameLine();
  ImGui::Checkbox("Auto-scroll", &auto_scroll_);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(200);
  ImGui::InputText("Filter", filter_buf_, sizeof(filter_buf_));
  ImGui::SameLine();
  if (ImGui::Button("+ New Plot")) {
    plotters_.emplace_back(next_plot_id_++, state_.series);
  }
  ImGui::Separator();

  ImGui::BeginChild("LogScroll", ImVec2(0, 0), ImGuiChildFlags_None,
                    ImGuiWindowFlags_HorizontalScrollbar);

  bool has_filter = filter_buf_[0] != '\0';

  for (auto &e : state_.entries) {
    if (has_filter) {
      bool match = e.tag.find(filter_buf_) != std::string::npos ||
                   e.message.find(filter_buf_) != std::string::npos;
      if (!match)
        continue;
    }

    float r, g, b, a;
    LevelColor(e.level, r, g, b, a);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r, g, b, a));

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

} // namespace debug
