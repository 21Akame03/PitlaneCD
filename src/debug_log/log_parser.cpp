#include "log_parser.hpp"

#include <charconv>
#include <cstring>
#include <sstream>
#include <cstdlib>

namespace DEBUG_LOG {

// ──────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────

LogLevel CharToLevel(char c) {
  switch (c) {
  case 'D':
    return LogLevel::DEBUG;
  case 'I':
    return LogLevel::INFO;
  case 'W':
    return LogLevel::WARN;
  case 'E':
    return LogLevel::ERROR;
  default:
    return LogLevel::UNKNOWN;
  }
}

void LevelColor(LogLevel lvl, float &r, float &g, float &b, float &a) {
  a = 1.0f;
  switch (lvl) {
  case LogLevel::DEBUG:
    r = 0.6f; g = 0.6f; b = 0.6f; break;
  case LogLevel::INFO:
    r = 0.4f; g = 0.8f; b = 1.0f; break;
  case LogLevel::WARN:
    r = 1.0f; g = 0.8f; b = 0.2f; break;
  case LogLevel::ERROR:
    r = 1.0f; g = 0.3f; b = 0.3f; break;
  default:
    r = 1.0f; g = 1.0f; b = 1.0f; break;
  }
}

// ──────────────────────────────────────────────
// Line parser
// Format: [timestamp] [DIWE] [tag] message | k=v k=v ...
// ──────────────────────────────────────────────

static bool TryParseDouble(const std::string &s, double &out) {
  // std::from_chars for double is not available on all toolchains yet,
  // fall back to strtod
  char *end = nullptr;
  out = std::strtod(s.c_str(), &end);
  return end != s.c_str() && *end == '\0';
}

static void ParseKVPairs(const std::string &kv_str,
                         std::unordered_map<std::string, double> &out) {
  std::istringstream ss(kv_str);
  std::string token;
  while (ss >> token) {
    auto eq = token.find('=');
    if (eq == std::string::npos || eq == 0 || eq == token.size() - 1)
      continue;
    std::string key = token.substr(0, eq);
    std::string val_str = token.substr(eq + 1);
    double val;
    if (TryParseDouble(val_str, val)) {
      out[key] = val;
    }
  }
}

bool ParseLine(const std::string &line, LogEntry &out) {
  // Manual parsing — much faster than std::regex.
  // Format: [timestamp] [DIWE] [tag] message | k=v k=v ...

  const char *p = line.c_str();

  // Skip leading whitespace
  while (*p == ' ' || *p == '\t') ++p;

  // Expect '['
  if (*p != '[') return false;
  ++p;

  // Parse timestamp digits
  const char *ts_start = p;
  while (*p >= '0' && *p <= '9') ++p;
  if (p == ts_start || *p != ']') return false;
  out.timestamp_ms = static_cast<uint32_t>(std::strtoul(ts_start, nullptr, 10));
  ++p; // skip ']'

  // Skip whitespace, expect '[', level char, ']'
  while (*p == ' ' || *p == '\t') ++p;
  if (*p != '[') return false;
  ++p;
  char lvl = *p;
  if (lvl != 'D' && lvl != 'I' && lvl != 'W' && lvl != 'E') return false;
  out.level = CharToLevel(lvl);
  ++p;
  if (*p != ']') return false;
  ++p;

  // Skip whitespace, expect '[', tag word chars, ']'
  while (*p == ' ' || *p == '\t') ++p;
  if (*p != '[') return false;
  ++p;
  const char *tag_start = p;
  while (*p && *p != ']') ++p;
  if (*p != ']' || p == tag_start) return false;
  out.tag.assign(tag_start, p);
  ++p; // skip ']'

  // Skip whitespace — rest is message [ | kv_pairs ]
  while (*p == ' ' || *p == '\t') ++p;

  std::string rest(p);
  auto pipe = rest.find('|');
  if (pipe != std::string::npos) {
    out.message = rest.substr(0, pipe);
    while (!out.message.empty() && out.message.back() == ' ')
      out.message.pop_back();
    ParseKVPairs(rest.substr(pipe + 1), out.values);
  } else {
    out.message = rest;
  }

  return true;
}

// ──────────────────────────────────────────────
// AppState
// ──────────────────────────────────────────────

void AppState::Clear() {
  entries.clear();
  series.clear();
}

void AppState::Feed(const std::string &raw_line) {
  LogEntry entry;
  if (!ParseLine(raw_line, entry))
    return;

  // For every numeric value, append to the corresponding PlotSeries
  for (auto &[key, val] : entry.values) {
    std::string series_name = entry.tag + "." + key;
    auto &s = series[series_name];
    if (s.name.empty())
      s.name = series_name;
    s.points.emplace_back(entry.timestamp_ms, val);
  }

  entries.push_back(std::move(entry));
}

} // namespace DEBUG_LOG
