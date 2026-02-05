#include "plotter.hpp"

#include <imgui.h>
#include <implot.h>
#include <math.h>
#include <vector>

namespace PLOTTER {

struct Buffer {
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  Buffer(int max_size = 2000) {
    MaxSize = max_size;
    Offset = 0;
    Data.reserve(max_size);
  }

  void AddPoint(float x, float y) {
    if (Data.size() < MaxSize) {
      Data.push_back(ImVec2(x, y));
    } else {
      Data[Offset] = ImVec2(x, y);
      Offset = (Offset + 1) % MaxSize;
    }
  }
};

Plotter_window::Plotter_window() {
  // Ensure an ImPlot context exists once; ImGui context is created in main.
  if (ImPlot::GetCurrentContext() == nullptr) {
    ImPlot::CreateContext();
  }
}

void Plotter_window::Plot_data(std::vector<float> x, std::vector<float> y) {}

void Plotter_window::RenderUI() {
  ImGui::Begin("Plotter Window");
  if (ImPlot::BeginPlot("Line Plots", ImVec2(-1, -1),
                        ImPlotAxisFlags_AutoFit |
                            ImPlotAxisFlags_NoGridLines)) {
    ImPlot::SetupAxes("x", "y");
    // ImPlot::PlotLine("f(x)", xs1, y1, 1001);
    // ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
    // ImPlot::PlotLine("g(x)", xs2, ys2, 20, ImPlotLineFlags_Segments);
    ImPlot::EndPlot();
  }
  ImGui::End();
}

} // namespace PLOTTER
