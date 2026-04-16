#include "debug/log_parser.hpp"

#include "nlohmann/json.hpp"

namespace debug {

LogLevel CharToLevel(char c) {
  switch (c) {
  case 'D':
    return LogLevel::DBG;
  case 'I':
    return LogLevel::INFO;
  case 'W':
    return LogLevel::WARN;
  case 'E':
    return LogLevel::ERR;
  default:
    return LogLevel::UNKNOWN;
  }
}

void LevelColor(LogLevel lvl, float &r, float &g, float &b, float &a) {
  a = 1.0f;
  switch (lvl) {
  case LogLevel::DBG:
    r = 0.6f; g = 0.6f; b = 0.6f; break;
  case LogLevel::INFO:
    r = 0.4f; g = 0.8f; b = 1.0f; break;
  case LogLevel::WARN:
    r = 1.0f; g = 0.8f; b = 0.2f; break;
  case LogLevel::ERR:
    r = 1.0f; g = 0.3f; b = 0.3f; break;
  default:
    r = 1.0f; g = 1.0f; b = 1.0f; break;
  }
}

// Expected JSON format: {"ts":12345,"lvl":"I","tag":"MOTOR","msg":"speed nominal","rpm":3200,"temp":85.3}
// Any numeric field besides ts/lvl/tag/msg is treated as a plottable value.
bool ParseLine(const std::string &line, LogEntry &out) {
  auto j = nlohmann::json::parse(line, nullptr, false);
  if (j.is_discarded())
    return false;

  if (!j.contains("ts") || !j.contains("lvl") || !j.contains("tag"))
    return false;

  out.timestamp_ms = j["ts"].get<uint32_t>();

  std::string lvl_str = j["lvl"].get<std::string>();
  out.level = lvl_str.empty() ? LogLevel::UNKNOWN : CharToLevel(lvl_str[0]);

  out.tag = j["tag"].get<std::string>();
  out.message = j.value("msg", "");

  for (auto &[key, val] : j.items()) {
    if (key == "ts" || key == "lvl" || key == "tag" || key == "msg")
      continue;
    if (val.is_number()) {
      out.values[key] = val.get<double>();
    }
  }

  return true;
}

void AppState::Clear() {
  entries.clear();
  series.clear();
}

void AppState::Feed(const std::string &raw_line) {
  LogEntry entry;
  if (!ParseLine(raw_line, entry))
    return;

  // Build plot series from key=value pairs, namespaced by tag
  for (auto &[key, val] : entry.values) {
    std::string series_name = entry.tag + "." + key;
    auto &s = series[series_name];
    if (s.name.empty())
      s.name = series_name;
    s.points.emplace_back(entry.timestamp_ms, val);
  }

  entries.push_back(std::move(entry));
}

} // namespace debug
