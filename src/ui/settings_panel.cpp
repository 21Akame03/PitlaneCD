#include "ui/settings_panel.hpp"
#include "ImGuiFileDialog.h"
#include "imgui.h"
#include <string>
#include <vector>

namespace ui {

static const char *DBC_DIALOG_KEY = "dbc_file_dialog";

SettingsPanel::SettingsPanel(serial::SerialReader &reader,
                             can::DbcParser &parser, AppMode &mode)
    : reader_(reader), parser_(parser), mode_(mode) {}

static const char *comport_combo_getter(void *data, int idx) {
  auto &v = *static_cast<std::vector<std::string> *>(data);
  if (idx < 0 || idx >= (int)v.size())
    return nullptr;
  return v[idx].c_str();
}

void SettingsPanel::connection_selector() {
  // Re-scan ports when nothing is selected yet.
  // Guard: only replace the list if we actually found ports — if the scan
  // returns nothing, keep the "Disconnected" placeholder so com_ports_ is
  // never empty and com_ports_[current_port_] is always a valid access.
  if (com_ports_.empty() || com_ports_[current_port_] == "Disconnected") {
    auto found = serial::list_serial_ports();
    if (!found.empty()) {
      com_ports_ = std::move(found);
      current_port_ = 0;
    }
  }

  ImGui::Combo("Interface", &current_port_, comport_combo_getter,
               (void *)&com_ports_, com_ports_.size());

  if (ImGui::Button(is_connected_ ? "Disconnect" : "Connect")) {
    is_connected_ = !is_connected_;
  }

  if (is_connected_) {
    reader_.Start(com_ports_[current_port_], 115200);
  } else {
    reader_.Stop();
  }
}

void SettingsPanel::dbc_selector() {
  ImGui::SeparatorText("DBC File");
  ImGui::TextWrapped("Select a .dbc file to decode CAN frames.");

  const bool has_selection = !dbc_file_path_.empty();
  ImGui::TextWrapped("Selected: %s",
                     has_selection ? dbc_file_path_.c_str() : "None");

  if (ImGui::Button("Browse##dbc")) {
    IGFD::FileDialogConfig config;

    if (has_selection) {
      const size_t split = dbc_file_path_.find_last_of("/\\");
      if (split != std::string::npos && split > 0) {
        config.path = dbc_file_path_.substr(0, split);
      }
    }

    if (config.path.empty())
      config.path = ".";

    ImGuiFileDialog::Instance()->OpenDialog(DBC_DIALOG_KEY, "Select DBC file",
                                            ".dbc", config);
  }

  if (has_selection && ImGui::SmallButton("Clear##dbc")) {
    dbc_file_path_.clear();
  }

  if (ImGuiFileDialog::Instance()->Display(
          DBC_DIALOG_KEY, ImGuiWindowFlags_None, ImVec2(600, 400),
          ImVec2(900, 600))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      dbc_file_path_ = ImGuiFileDialog::Instance()->GetFilePathName();
      parser_.request_load(dbc_file_path_);
    }
    ImGuiFileDialog::Instance()->Close();
  }
}

void SettingsPanel::render_ui() {
  ImGui::Begin("Settings Window");

  connection_selector();

  if (mode_ == AppMode::CanSniffer) {
    dbc_selector();
  }

  ImGui::End();
}

} // namespace ui
