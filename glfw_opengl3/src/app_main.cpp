#include "app_main.hpp"
#include "imgui.h"

namespace MyApp {
void RenderUI() {
  ImGui::Begin("MyApp");
  ImGui::Button("Click me");
  static float f = 0.0f;
  ImGui::DragFloat("float", &f);
  ImGui::End();
};
} // namespace MyApp
