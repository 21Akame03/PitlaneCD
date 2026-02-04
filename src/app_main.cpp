#include "app_main.hpp"
#include "CAN_sniffer/can_sniffer.hpp"
#include "imgui.h"
#include "implot.h"
#include "plotter/plotter.hpp"
#include "settings/settings.hpp"

#include <iostream>
#include <vector>

// NOTE: Variable list
namespace MyApp {

bool demo = false;

std::vector<SETTINGS::VariableCheckbox> variables;
CAN_SNIFFER_WINDOW::Sniffer_window sniffer =
    CAN_SNIFFER_WINDOW::Sniffer_window();

// Mode
// 1. Plotter / Debug Mode
// 2. Can_sniffer Mode
// 3. Telemetry Mode
Mode_t mode = Mode_t::NONE;

/* -----------------------------------------
 * Purpose: At the start if the Program, asks the user to select a mode using a
 * Popup window
 * Input : NONE
 * Output: NONE
 */

void ModeSelector() {
  if (ImGui::BeginPopupModal("mode?", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("This Program can be used in 3 Different Modes\nYou need to "
                "choose one of them before you can proceed!");
    ImGui::Separator();

    // static int unused_i = 0;
    // ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

    static bool dont_ask_me_next_time = false;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::Checkbox("Select Mode: ", &dont_ask_me_next_time);
    ImGui::PopStyleVar();

    if (ImGui::Button("Debug", ImVec2(120, 0))) {
      MyApp::mode = Mode_t::DEBUG;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("CAN Sniffer", ImVec2(120, 0))) {
      MyApp::mode = Mode_t::CAN_SNIFFER;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  ImGui::OpenPopup("mode?");
}

void RenderUI() {
  // TL;DR; this demo is more complicated than what most users you would
  // normally use. If we remove all options we are showcasing, this demo would
  // become a simple call to ImGui::DockSpaceOverViewport() !! In this specific
  // demo, we are not using DockSpaceOverViewport() because:

  static bool opt_fullscreen = true;
  static bool opt_padding = false;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window
  // not dockable into, because it would be confusing to have two docking
  // targets within each others.
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen) {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |=
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  } else {
    dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
  }

  // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render
  // our background and handle the pass-thru hole, so we ask Begin() to not
  // render a background.
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  // Important: note that we proceed even if Begin() returns false (aka window
  // is collapsed). This is because we want to keep our DockSpace() active. If a
  // DockSpace() is inactive, all active windows docked into it will lose their
  // parent and become undocked. We cannot preserve the docking relationship
  // between an active window and an inactive docking, otherwise any change of
  // dockspace/settings would lead to windows being stuck in limbo and never
  // being visible.
  if (!opt_padding)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", nullptr, window_flags);
  if (!opt_padding)
    ImGui::PopStyleVar();

  if (opt_fullscreen)
    ImGui::PopStyleVar(2);

  // Submit the DockSpace
  // REMINDER: THIS IS A DEMO FOR ADVANCED USAGE OF DockSpace()!
  // MOST REGULAR APPLICATIONS WILL SIMPLY WANT TO CALL DockSpaceOverViewport().
  // READ COMMENTS ABOVE.
  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  // Show demo options and help
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Options")) {
      // Disabling fullscreen would allow the window to be moved to the front of
      // other windows, which we can't undo at the moment without finer window
      // depth/z control.
      ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
      ImGui::MenuItem("Padding", NULL, &opt_padding);
      ImGui::Separator();

      if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "",
                          (dockspace_flags &
                           ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode;
      }
      if (ImGui::MenuItem(
              "Flag: NoDockingSplit", "",
              (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit;
      }
      if (ImGui::MenuItem("Flag: NoUndocking", "",
                          (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) !=
                              0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking;
      }
      if (ImGui::MenuItem("Flag: NoResize", "",
                          (dockspace_flags & ImGuiDockNodeFlags_NoResize) !=
                              0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
      }
      if (ImGui::MenuItem(
              "Flag: AutoHideTabBar", "",
              (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) {
        dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
      }
      if (ImGui::MenuItem(
              "Flag: PassthruCentralNode", "",
              (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0,
              opt_fullscreen)) {
        dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
      }
      ImGui::Separator();

      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }

  // Threads are made for
  // 1. Serial Comm port reader and saving the data
  // 2. The plotter which will take the data and show it on the Settings windows

  if (MyApp::mode == Mode_t::NONE) {
    ModeSelector();
  }

  // render Settings Window
  SETTINGS::RenderUI();

  // based on the mode selection, we choose a mode.
  switch (MyApp::mode) {
    // Mode Selector should still be there

  // Debug Mode
  case Mode_t::DEBUG:

    break;

  // renders the CAN_SNIFFER VIEW
  case Mode_t::CAN_SNIFFER:
    sniffer.RenderUI();
    break;

  case Mode_t::NONE:
  default:
    break;
  }

  // Just leave it here
  if (demo) {
    ImGui::ShowDemoWindow();
  }
  ImGui::End();
};
} // namespace MyApp
