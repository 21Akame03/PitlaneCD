#include "can/sniffer_panel.hpp"
#include "imgui.h"
#include "implot.h"
#include <algorithm>
#include <cstdio>
#include <deque>
#include <map>
#include <vector>

namespace can {

SnifferPanel::SnifferPanel(DbcParser &parser, AppLog &log,
                           serial::SerialReader &reader)
    : parser_(parser), log_(log), reader_(reader) {}

void SnifferPanel::render_signal_bar_plot(
    const char *plotTitle,
    const std::vector<std::pair<uint32_t, const SignalValue *>> &signals) {
  if (signals.empty())
    return;

  std::vector<double> positions;
  std::vector<double> values;
  for (int i = 0; i < (int)signals.size(); i++) {
    positions.push_back(i);
    values.push_back(signals[i].second->physicalValue);
  }

  static const double barWidth = 0.6;
  float plotHeight = 120.0f;
  if (ImPlot::BeginPlot(plotTitle, ImVec2(-1, plotHeight))) {
    ImPlot::SetupAxes(nullptr, "Value");
    ImPlot::SetupAxis(ImAxis_X1, nullptr,
                      ImPlotAxisFlags_NoTickLabels |
                          ImPlotAxisFlags_NoTickMarks);
    ImPlot::SetupAxisLimits(ImAxis_X1, -0.5, (double)signals.size() - 0.5,
                            ImPlotCond_Always);

    double yMin = 0, yMax = 0;
    for (double v : values) {
      if (v < yMin)
        yMin = v;
      if (v > yMax)
        yMax = v;
    }
    double yPad = (yMax - yMin) * 0.1;
    if (yPad < 1.0)
      yPad = 1.0;
    ImPlot::SetupAxisLimits(ImAxis_Y1, yMin - yPad, yMax + yPad,
                            ImPlotCond_Always);

    ImPlot::PlotBars("Signals", positions.data(), values.data(),
                     (int)signals.size(), barWidth);

    if (ImPlot::IsPlotHovered()) {
      ImPlotPoint mouse = ImPlot::GetPlotMousePos();
      int idx = (int)(mouse.x + 0.5);
      if (idx >= 0 && idx < (int)signals.size()) {
        double barLeft = positions[idx] - barWidth / 2.0;
        double barRight = positions[idx] + barWidth / 2.0;
        if (mouse.x >= barLeft && mouse.x <= barRight) {
          ImGui::BeginTooltip();
          ImGui::Text("Signal: %s", signals[idx].second->name.c_str());
          ImGui::Text("Value:  %.2f %s", signals[idx].second->physicalValue,
                      signals[idx].second->unit.c_str());
          ImGui::Text("Raw:    0x%X", signals[idx].second->rawValue);
          ImGui::Text("Msg ID: 0x%X", signals[idx].first);
          ImGui::EndTooltip();
        }
      }
    }
    ImPlot::EndPlot();
  }
}

void SnifferPanel::render_bar_plot() {
  struct MuxMsgSignals {
    const MessageData *msg;
    std::vector<std::pair<uint32_t, const SignalValue *>> sigs;
  };
  std::map<uint32_t, MuxMsgSignals> multiplexed_msgs;
  std::vector<std::pair<uint32_t, const SignalValue *>> non_mux_signals;

  for (const auto &entry : parser_.signal_store()) {
    const auto &msgData = entry.second;
    for (const auto &sig : msgData.signals) {
      if (sig.second.isMultiplexed) {
        auto &mux = multiplexed_msgs[msgData.id];
        mux.msg = &msgData;
        mux.sigs.push_back({msgData.id, &sig.second});
      } else {
        non_mux_signals.push_back({msgData.id, &sig.second});
      }
    }
  }

  for (auto &[id, muxMsg] : multiplexed_msgs) {
    std::sort(muxMsg.sigs.begin(), muxMsg.sigs.end(),
              [](const auto &a, const auto &b) {
                return a.second->name < b.second->name;
              });
    char title[128];
    snprintf(title, sizeof(title), "%s (0x%X)", muxMsg.msg->name.c_str(),
             muxMsg.msg->id);
    render_signal_bar_plot(title, muxMsg.sigs);
  }

  render_signal_bar_plot("Non-Multiplexed Signals", non_mux_signals);
}

void SnifferPanel::render_ui() {
  ImGui::Begin("CAN Sniffer Window");

  // Poll serial and parse frames
  auto lines = reader_.PollRxBuffer();
  for (const auto &line : lines) {
    parser_.parse_frame(line);
  }

  render_bar_plot();
  log_.Draw("Log");

  ImGui::End();
}

} // namespace can
