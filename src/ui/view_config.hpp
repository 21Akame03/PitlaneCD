#ifndef VIEW_CONFIG_HPP
#define VIEW_CONFIG_HPP

#include "ui/settings_panel.hpp"
#include <set>
#include <string>
#include <vector>

namespace ui {

struct PlotConfig {
  std::set<std::string> enabled_series;
  float history_secs = 10.0f;
  bool follow = true;
  bool auto_fit = false;
  bool show_max = false;
  bool show_min = false;
};

struct ViewConfig {
  AppMode mode = AppMode::None;
  std::string dbc_path;
  std::vector<PlotConfig> plots;
};

bool path_exists(const std::string &path);
bool load_view_config(const std::string &path, ViewConfig &out);
bool save_view_config(const std::string &path, const ViewConfig &cfg);

// "Last used" layout path, persisted to a small pointer file next to the exe.
std::string load_last_view_path();
bool save_last_view_path(const std::string &path);

} // namespace ui

#endif
