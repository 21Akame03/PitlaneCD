#include "settings.hpp"
#include "ImGuiFileDialog.h"
#include "Serial_reader/serial_inputs.hpp"
#include "app_main.hpp"
#include "imgui.h"
#include "plotter/plotter.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

namespace SETTINGS {

// Connection parameters (initially contains Disconnected as a state)
std::vector<std::string> com_ports = {"Disconnected"};
static int current_port = 0;
static bool ispollActive = false;
static std::string dbc_file_path;
static const char *DBC_DIALOG_KEY = "dbc_file_dialog";

// Serial Reader object
static SERIAL::SerialReader serialReader;

void createCheckboxes(std::vector<VariableCheckbox> &variables) {
  // we creake the checkboxes
  for (auto &var : variables) {
    ImGui::Checkbox(var.VariableName.c_str(), &var.isChecked);
  }
}

// Ask for type of data which will be received from serial port / Wifi
// 1. Debug data
// 2. Json for CAN_Sniffer

/*
 * Purpose: getter function for the com_port combo
 * Input: pointer to vector of strings, index of the string to be returned and
 * the string itself
 * Output: operation status
 */
bool comport_combo_getter(void *data, int idx, const char **out_text) {
  auto &v = *static_cast<std::vector<std::string> *>(data);
  if (idx < 0 || idx >= (int)v.size())
    return false;
  *out_text = v[idx].c_str();
  return true;
}

/*
 * Purpose: Create a combo for selecting the serial port
 * Input: None
 * Output: None
 */
void connection_selector() {

  // poll all available serial ports
  if (com_ports[current_port] == "Disconnected") {
    com_ports = listSerialPorts();
  }

  // take the values from the vector and ppopulate the combo
  ImGui::Combo("Interface", &current_port, comport_combo_getter,
               (void *)&com_ports, com_ports.size());

  // If ispollActive is true, show disconnect else show Connect
  if (ImGui::Button(ispollActive ? "Disconnect" : "Connect")) {
    ispollActive = !ispollActive;
  }

  // So long as ispollActive is true, start the serial reader
  if (ispollActive) {
    serialReader.Start(com_ports[current_port], 115200);
  } else {
    serialReader.Stop();
  }
}

/*
 * Purpose: Select the DBC file using a file dialog
 */
void dbc_selector() {
  ImGui::SeparatorText("DBC File");
  ImGui::TextWrapped("Select a .dbc file to decode CAN frames.");

  const bool has_selection = !dbc_file_path.empty();
  ImGui::TextWrapped("Selected: %s",
                     has_selection ? dbc_file_path.c_str() : "None");

  if (ImGui::Button("Browse##dbc")) {
    IGFD::FileDialogConfig config;

    // Re-open the dialog in the last used directory when possible
    if (has_selection) {
      const size_t split = dbc_file_path.find_last_of("/\\");
      if (split != std::string::npos && split > 0) {
        config.path = dbc_file_path.substr(0, split);
      }
    }

    if (config.path.empty())
      config.path = ".";

    ImGuiFileDialog::Instance()->OpenDialog(DBC_DIALOG_KEY, "Select DBC file",
                                            ".dbc", config);
  }

  // ImGui::SameLine();
  if (has_selection && ImGui::SmallButton("Clear##dbc")) {
    dbc_file_path.clear();
  }

  if (ImGuiFileDialog::Instance()->Display(DBC_DIALOG_KEY)) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      dbc_file_path = ImGuiFileDialog::Instance()->GetFilePathName();
    }
    ImGuiFileDialog::Instance()->Close();
  }
}

void RenderUI() {
  ImGui::Begin("Settings Window");

  connection_selector();

  if (MyApp::mode == MyApp::CAN_SNIFFER) {
    // UI for the File Dialog
    dbc_selector();

    // Data Parser through CAN
  }

  ImGui::Separator();
  ImGui::Text("Available Variables:");

  if (ImGui::BeginListBox("##listbox Variables", ImVec2(-FLT_MIN, -1))) {
    createCheckboxes(MyApp::variables);
  }
  ImGui::EndListBox();

  ImGui::End();
};
} // namespace SETTINGS
