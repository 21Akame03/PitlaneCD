#include "settings.hpp"
#include "app_main.hpp"
#include "imgui.h"
#include "plotter/plotter.hpp"
#include "stdio.h"
#include "vector"

namespace SETTINGS {

void createCheckboxes(std::vector<VariableCheckbox> &variables) {
  // we creake the checkboxes
  for (auto &var : variables) {
    ImGui::Checkbox(var.VariableName.c_str(), &var.isChecked);
  }
}

void RenderUI() {
  ImGui::Begin("Settings Window");

  ImGui::Text("Available Variables:");
  createCheckboxes(MyApp::variables);
  ImGui::End();
}
} // namespace SETTINGS
