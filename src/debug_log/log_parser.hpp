#ifndef LOG_PARSER_HPP
#define LOG_PARSER_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace DEBUG_LOG {

enum class LogLevel { DEBUG, INFO, WARN, ERROR, UNKNOWN };

struct LogEntry {
  uint32_t timestamp_ms;
  LogLevel level;
  std::string tag;
  std::string message;
  std::unordered_map<std::string, double> values; // e.g. {"throttle": 84.3}
};

struct PlotSeries {
  std::string name;                        // "APPS.throttle"
  std::vector<std::pair<uint32_t, double>> points; // (timestamp_ms, value)
};

struct AppState {
  std::vector<LogEntry> entries;
  std::unordered_map<std::string, PlotSeries> series; // keyed by "TAG.key"

  void Clear();
  void Feed(const std::string &raw_line);
};

// Parse a single raw line into a LogEntry. Returns false if the line
// doesn't match the expected format.
bool ParseLine(const std::string &line, LogEntry &out);

// Convert a single character level code to LogLevel enum
LogLevel CharToLevel(char c);

// Get ImGui colour for a given log level (returns ImVec4-compatible RGBA)
void LevelColor(LogLevel lvl, float &r, float &g, float &b, float &a);

} // namespace DEBUG_LOG

#endif // LOG_PARSER_HPP
