#include "settings.hpp"
#include "app_main.hpp"
#include "imgui.h"
#include "plotter/plotter.hpp"
#include "stdio.h"
#include "vector"
#include <iostream>

namespace SETTINGS {

void createCheckboxes(std::vector<VariableCheckbox> &variables) {
  // we creake the checkboxes
  for (auto &var : variables) {
    ImGui::Checkbox(var.VariableName.c_str(), &var.isChecked);
  }
}

// Ask for type of data which will be received from serial port / Wifi
// 1. Debug data
// 2. Json for CAN_Sniffer

void RenderUI() {
  ImGui::Begin("Settings Window");

  ImGui::Text("Available Variables:");
  ImGui::Separator();

  // Custom size: use all width, 5 items tall
  if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, -1))) {
    // for (int n = 0; n < IM_COUNTOF(items); n++) {
    //   bool is_selected = (item_selected_idx == n);
    //   ImGuiSelectableFlags flags =
    //       (item_highlighted_idx == n) ? ImGuiSelectableFlags_Highlight : 0;
    //   if (ImGui::Selectable(items[n], is_selected, flags))
    //     item_selected_idx = n;
    //
    // Set the initial focus when opening the combo (scrolling + keyboard
    // navigation focus)
    // if (is_selected)
    //   ImGui::SetItemDefaultFocus();
    createCheckboxes(MyApp::variables);
  }
  ImGui::EndListBox();

  ImGui::End();
}
} // namespace SETTINGS
