#include "can_sniffer.hpp"
#include "imgui.h"
#include "implot.h"

/*
 * Shares similar viewport as normal Debug Plotter with some additions:
 * 1. adds a console to print the values in text
 * 2. uses a bar plot or maybe a line graph for specific signals
 * 3. has a section where user has to provide a DBC file for decryption
 */

namespace CAN_SNIFFER_WINDOW {

/*
 * --------------------------------------------------------
 * Purpose: Creates a Bar/histogram to show the current values of the decrypted
 * value Input: NONE
 * Output: NONE
 * --------------------------------------------------------
 */
void Bar_plot() {
  // Test data for visualisation test
  static ImS8 data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  if (ImPlot::BeginPlot("CAN Messages")) {
    ImPlot::PlotBars("Value", data, 10, 0, 7, 1);
    ImPlot::EndPlot();
  }
}

Sniffer_window::Sniffer_window() {
  // Ensure an ImPlot context exists once; ImGui context is created in main.
  if (ImPlot::GetCurrentContext() == nullptr) {
    ImPlot::CreateContext();
  }
}

/*
 * --------------------------------------------------------
 * Purpose: Renders the UI for CAN_Sniffer
 * value Input: NONE
 * Output: NONE
 * --------------------------------------------------------
 */
void Sniffer_window::RenderUI() {
  ImGui::Begin("CAN Sniffer Window");
  Bar_plot();
  ImGui::End();
}

} // namespace CAN_SNIFFER_WINDOW
