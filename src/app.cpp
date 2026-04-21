#include "ImGuiFileDialog.h"
#include "app.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ui/settings_panel.hpp"

static const char *LOAD_VIEW_DIALOG_KEY = "load_view_dialog";

// USER CODE BEGINS AT THE BOTTOM
// DO NOT CHANGE ANYTHING WITH DOCKING

App::App()
    : sniffer(dbc_parser, log, serial_reader, mdf_logger),
      ig(dbc_parser, serial_reader),
      debug(serial_reader, mdf_logger, mode),
      settings(serial_reader, dbc_parser, mode, mdf_logger) {
  dbc_parser.set_log_callback(
      [this](const std::string &msg) { log.AddLog("%s", msg.c_str()); });
  mdf_logger.set_log_callback(
      [this](const std::string &msg) { log.AddLog("%s\n", msg.c_str()); });
  settings.set_on_new_plot([this]() { debug.add_plot(); });
  settings.set_on_save_view(
      [this](const std::string &path) { save_current_view(path); });

  last_view_path_ = ui::load_last_view_path();
  view_prompt_pending_ = true;
}

void App::apply_view_config() {
  mode = pending_view_.mode;
  if (!pending_view_.dbc_path.empty()) {
    settings.set_dbc_path(pending_view_.dbc_path);
    dbc_parser.request_load(pending_view_.dbc_path);
  }
  for (auto &pc : pending_view_.plots) {
    debug.add_plot(pc);
  }
}

void App::save_current_view(const std::string &path) {
  ui::ViewConfig cfg;
  cfg.mode = mode;
  cfg.dbc_path = settings.dbc_path();
  cfg.plots = debug.export_plot_configs();
  if (ui::save_view_config(path, cfg)) {
    ui::save_last_view_path(path);
    last_view_path_ = path;
    settings.flash_saved();
    log.AddLog("View saved to %s\n", path.c_str());
  } else {
    log.AddLog("Failed to save view config to %s\n", path.c_str());
  }
}

void App::load_view_from(const std::string &path) {
  if (ui::load_view_config(path, pending_view_)) {
    apply_view_config();
    ui::save_last_view_path(path);
    last_view_path_ = path;
    log.AddLog("Loaded view from %s\n", path.c_str());
  } else {
    log.AddLog("Failed to load view from %s\n", path.c_str());
  }
  view_prompt_pending_ = false;
}

void App::view_prompt() {
  const bool have_last = !last_view_path_.empty() &&
                         ui::path_exists(last_view_path_);

  if (!view_browsing_) {
    if (ImGui::BeginPopupModal("Open project layout", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Choose a saved project layout, or start a new session.");
      ImGui::Separator();

      ImGui::BeginDisabled(!have_last);
      if (ImGui::Button("Use last", ImVec2(140, 0))) {
        load_view_from(last_view_path_);
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndDisabled();
      if (have_last) {
        ImGui::SameLine();
        ImGui::TextDisabled("(%s)", last_view_path_.c_str());
      }

      if (ImGui::Button("Browse...", ImVec2(140, 0))) {
        IGFD::FileDialogConfig config;
        config.path = have_last ? last_view_path_.substr(
                                      0, last_view_path_.find_last_of("/\\"))
                                : std::string(".");
        if (config.path.empty()) config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog(
            LOAD_VIEW_DIALOG_KEY, "Select view layout", ".json", config);
        view_browsing_ = true;
        ImGui::CloseCurrentPopup();
      }

      if (ImGui::Button("Start new", ImVec2(140, 0))) {
        view_prompt_pending_ = false;
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }
    ImGui::OpenPopup("Open project layout");
  }

  if (view_browsing_ &&
      ImGuiFileDialog::Instance()->Display(
          LOAD_VIEW_DIALOG_KEY, ImGuiWindowFlags_None, ImVec2(600, 400),
          ImVec2(900, 600))) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      load_view_from(ImGuiFileDialog::Instance()->GetFilePathName());
    }
    ImGuiFileDialog::Instance()->Close();
    view_browsing_ = false;
  }
}

// Only runs once — after that imgui.ini takes over
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

// ----------------------------
// USER CODE BEGINS HERE
// ----------------------------
void App::mode_selector() {
  if (ImGui::BeginPopupModal("mode?", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("This Program can be used in 3 Different Modes\n"
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
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("Telemetry", ImVec2(120, 0))) {
      mode = ui::AppMode::Telemetry;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  ImGui::OpenPopup("mode?");
}
// ----------------------------
// USER CODE ENDS HERE
// ----------------------------

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

  // ----------------------------
  // USER CODE BEGINS HERE
  // ----------------------------
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Options")) {
      ImGui::MenuItem("Fullscreen", NULL, nullptr);
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  if (view_prompt_pending_) {
    view_prompt();
  } else if (mode == ui::AppMode::None) {
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
    case ui::AppMode::Telemetry:
    debug.render_ui();
    break;
  case ui::AppMode::None:
  default:
    break;
  }

  ImGui::End();
}
