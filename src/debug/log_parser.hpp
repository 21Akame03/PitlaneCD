#ifndef DEBUG_LOG_PARSER_HPP
#define DEBUG_LOG_PARSER_HPP

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace debug {

enum class LogLevel { DBG, INFO, WARN, ERR, UNKNOWN };

struct LogEntry {
  uint32_t timestamp_ms;
  LogLevel level;
  std::string tag;
  std::string message;
  std::unordered_map<std::string, double> values;
};

struct PlotSeries {
  std::string name;
  std::vector<std::pair<uint32_t, double>> points;
};

struct AppState {
  std::vector<LogEntry> entries;
  std::unordered_map<std::string, PlotSeries> series;

  void Clear();
  void Feed(const std::string &raw_line);
};

bool ParseLine(const std::string &line, LogEntry &out);
LogLevel CharToLevel(char c);
void LevelColor(LogLevel lvl, float &r, float &g, float &b, float &a);

} // namespace debug

#endif // DEBUG_LOG_PARSER_HPP
