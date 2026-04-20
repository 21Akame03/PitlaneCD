#ifndef DBC_PARSER_HPP
#define DBC_PARSER_HPP

#include "nlohmann/json.hpp"
#include <Vector/DBC/Network.h>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace can {

struct CanFrame {
  uint32_t id;
  uint8_t dlc;
  double value;
  std::string name;
};

struct SignalValue {
  std::string name;
  double physicalValue;
  unsigned int rawValue;
  std::string unit;
  bool isMultiplexed;
};

struct MessageData {
  std::string name;
  uint32_t id;
  bool hasMultiplexedSignals = false;
  std::map<std::string, SignalValue> signals;
};

class DbcParser {
public:
  using LogCallback = std::function<void(const std::string &)>;

  explicit DbcParser();

  void set_log_callback(LogCallback cb) { log_cb_ = std::move(cb); }

  bool load_dbc(const std::string &filepath);
  bool request_load(const std::string &path);
  bool is_loaded() const { return loaded_; }
  std::optional<CanFrame> parse_frame(const std::string &data);
  const std::map<uint32_t, Vector::DBC::Message> *getMessages() const {
    return loaded_ ? &net_->messages : nullptr;
  }
  const std::map<uint32_t, MessageData> &signal_store() const {
    return signal_store_;
  }

private:
  void log(const char *fmt, ...);
  nlohmann::json parse_json(const std::string &data);
  std::vector<uint8_t> hex_to_bytes(const std::string &hex);
  std::optional<Vector::DBC::Message> find_message(uint32_t id);

  std::unique_ptr<Vector::DBC::Network> net_;
  bool loaded_ = false;
  std::string dbcfilepath_;
  LogCallback log_cb_;
  std::map<uint32_t, MessageData> signal_store_;
};

} // namespace can

#endif // DBC_PARSER_HPP
