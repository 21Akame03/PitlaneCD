#include "app.hpp"
#include "imgui.h"
#include "imgui_internal.h"

App::App()
    : sniffer(dbc_parser, log, serial_reader),
      ig(dbc_parser, serial_reader),
      debug(serial_reader),
      settings(serial_reader, dbc_parser, mode) {
  dbc_parser.set_log_callback(
      [this](const std::string &msg) { log.AddLog("%s", msg.c_str()); });
}

void App::setup_default_docking_layout(unsigned int dockspace_id) {
  ImGuiID id = dockspace_id;
  ImGuiDockNode *node = ImGui::DockBuilderGetNode(id);
  if (node != nullptr && node->IsSplitNode()) {
    first_time_layout_ = false;
    return;
  }

  first_time_layout_ = false;

  ImGui::DockBuilderRemoveNode(id);
  ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(id, ImGui::GetMainViewport()->Size);

  ImGuiID dock_main_id = id;
  ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(
      dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
  ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(
      dock_main_id, ImGuiDir_Down, 0.2f, nullptr, &dock_main_id);

  ImGui::DockBuilderDockWindow("Settings Window", dock_right_id);
  ImGui::DockBuilderDockWindow("CAN Sniffer Window", dock_main_id);
  ImGui::DockBuilderDockWindow("Plotter Window", dock_main_id);
  ImGui::DockBuilderDockWindow("Log", dock_bottom_id);

  ImGui::DockBuilderFinish(id);
}

void App::mode_selector() {
  if (ImGui::BeginPopupModal("mode?", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("This Program can be used in 2 Different Modes\n"
                "You need to choose one before you can proceed!");
    ImGui::Separator();

    if (ImGui::Button("Debug", ImVec2(120, 0))) {
      mode = ui::AppMode::Debug;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("CAN Sniffer", ImVec2(120, 0))) {
      mode = ui::AppMode::CanSniffer;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  ImGui::OpenPopup("mode?");
}

void App::RenderUI() {
  // Fullscreen dockspace
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace", nullptr, window_flags);
  ImGui::PopStyleVar(3);

  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

    if (first_time_layout_) {
      setup_default_docking_layout(dockspace_id);
    }
  }

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Options")) {
      ImGui::MenuItem("Fullscreen", NULL, nullptr);
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  if (mode == ui::AppMode::None) {
    mode_selector();
  }

  settings.render_ui();

  switch (mode) {
  case ui::AppMode::Debug:
    debug.render_ui();
    break;
  case ui::AppMode::CanSniffer:
    sniffer.render_ui();
    ig.render_ui();
    break;
  case ui::AppMode::None:
  default:
    break;
  }

  ImGui::End();
}
