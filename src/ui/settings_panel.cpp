#include "ui/settings_panel.hpp"
#include "ImGuiFileDialog.h"
#include "imgui.h"
#include <string>
#include <vector>

namespace ui {

static const char *DBC_DIALOG_KEY = "dbc_file_dialog";
static const char *SAVE_VIEW_DIALOG_KEY = "save_view_dialog";

SettingsPanel::SettingsPanel(serial::SerialReader &reader,
                             can::DbcParser &parser, AppMode &mode,
                             logging::MdfLogger &mdf)
    : reader_(reader), parser_(parser), mode_(mode), mdf_(mdf) {}

// Auto-start on connect / auto-stop on disconnect. User can override with
// the manual Start/Stop button in recording_controls().
void SettingsPanel::auto_start_logging() {
  if (is_connected_ && !was_connected_) {
    if (mode_ == AppMode::CanSniffer) mdf_.start_can();
    else if (mode_ == AppMode::Telemetry) mdf_.start_telemetry();
  } else if (!is_connected_ && was_connected_) {
    if (mdf_.is_recording()) mdf_.stop();
  }
  was_connected_ = is_connected_;
}

void SettingsPanel::recording_controls() {
  if (mode_ != AppMode::CanSniffer && mode_ != AppMode::Telemetry) return;

  ImGui::SeparatorText("Recording (MDF4)");

  const bool recording = mdf_.is_recording();
  if (recording) {
    if (ImGui::Button("Stop Recording")) {
      mdf_.stop();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("%s", mdf_.current_path().c_str());
  } else {
    const char *label = mode_ == AppMode::CanSniffer ? "Start Recording (CAN)"
                                                      : "Start Recording";
    if (ImGui::Button(label)) {
      if (mode_ == AppMode::CanSniffer) mdf_.start_can();
      else mdf_.start_telemetry();
    }
  }
}

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

  auto_start_logging();
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

  if (mode_ == AppMode::Debug || mode_ == AppMode::Telemetry) {
    ImGui::SeparatorText("Plots");
    if (ImGui::Button("+ Plot") && on_new_plot_) {
      on_new_plot_();
    }
  }

  recording_controls();

  if (mode_ != AppMode::None) {
    ImGui::SeparatorText("View");
    if (ImGui::Button("Save View As...")) {
      IGFD::FileDialogConfig config;
      config.path = ".";
      config.fileName = "view.json";
      config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;
      ImGuiFileDialog::Instance()->OpenDialog(
          SAVE_VIEW_DIALOG_KEY, "Save View As", ".json", config);
    }
    if (saved_feedback_timer_ > 0.0f) {
      ImGui::SameLine();
      ImGui::TextDisabled("Saved.");
      saved_feedback_timer_ -= ImGui::GetIO().DeltaTime;
    }

    if (ImGuiFileDialog::Instance()->Display(
            SAVE_VIEW_DIALOG_KEY, ImGuiWindowFlags_None, ImVec2(600, 400),
            ImVec2(900, 600))) {
      if (ImGuiFileDialog::Instance()->IsOk() && on_save_view_) {
        on_save_view_(ImGuiFileDialog::Instance()->GetFilePathName());
      }
      ImGuiFileDialog::Instance()->Close();
    }
  }

  ImGui::End();
}

} // namespace ui
