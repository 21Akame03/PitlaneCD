#include "ui/view_config.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sys/stat.h>

namespace ui {

using nlohmann::json;

static const char *LAST_PATH_FILE = ".pitlane_last_view";

static const char *mode_to_str(AppMode m) {
  switch (m) {
  case AppMode::CanSniffer: return "can_sniffer";
  case AppMode::Debug: return "debug";
  case AppMode::Telemetry: return "telemetry";
  case AppMode::None:
  default: return "none";
  }
}

static AppMode mode_from_str(const std::string &s) {
  if (s == "can_sniffer") return AppMode::CanSniffer;
  if (s == "debug") return AppMode::Debug;
  if (s == "telemetry") return AppMode::Telemetry;
  return AppMode::None;
}

bool path_exists(const std::string &path) {
  if (path.empty()) return false;
  struct stat st;
  return ::stat(path.c_str(), &st) == 0;
}

bool load_view_config(const std::string &path, ViewConfig &out) {
  std::ifstream f(path);
  if (!f.is_open()) return false;
  json j;
  try {
    f >> j;
  } catch (...) {
    return false;
  }

  out.mode = mode_from_str(j.value("mode", std::string("none")));
  out.dbc_path = j.value("dbc_path", std::string());
  out.plots.clear();
  if (j.contains("plots") && j["plots"].is_array()) {
    for (auto &p : j["plots"]) {
      PlotConfig pc;
      pc.history_secs = p.value("history_secs", 10.0f);
      pc.follow = p.value("follow", true);
      pc.auto_fit = p.value("auto_fit", false);
      pc.show_max = p.value("show_max", false);
      pc.show_min = p.value("show_min", false);
      if (p.contains("series") && p["series"].is_array()) {
        for (auto &s : p["series"]) {
          if (s.is_string()) pc.enabled_series.insert(s.get<std::string>());
        }
      }
      out.plots.push_back(std::move(pc));
    }
  }
  return true;
}

bool save_view_config(const std::string &path, const ViewConfig &cfg) {
  json j;
  j["mode"] = mode_to_str(cfg.mode);
  j["dbc_path"] = cfg.dbc_path;
  j["plots"] = json::array();
  for (auto &p : cfg.plots) {
    json jp;
    jp["history_secs"] = p.history_secs;
    jp["follow"] = p.follow;
    jp["auto_fit"] = p.auto_fit;
    jp["show_max"] = p.show_max;
    jp["show_min"] = p.show_min;
    jp["series"] = json::array();
    for (auto &s : p.enabled_series) jp["series"].push_back(s);
    j["plots"].push_back(std::move(jp));
  }
  std::ofstream f(path);
  if (!f.is_open()) return false;
  f << j.dump(2);
  return f.good();
}

std::string load_last_view_path() {
  std::ifstream f(LAST_PATH_FILE);
  if (!f.is_open()) return {};
  std::string line;
  std::getline(f, line);
  return line;
}

bool save_last_view_path(const std::string &path) {
  std::ofstream f(LAST_PATH_FILE);
  if (!f.is_open()) return false;
  f << path;
  return f.good();
}

} // namespace ui
