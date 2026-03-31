#include "serial/serial_reader.hpp"
#include <string>
#include <vector>

#if defined(_WIN32)
#include <devguid.h>
#include <regstr.h>
#include <setupapi.h>
#include <windows.h>
#pragma comment(lib, "setupapi.lib")
#elif defined(__APPLE__) || defined(__linux__)
#include <filesystem>
#endif

namespace serial {

#if defined(_WIN32)
static std::vector<std::string> list_serial_ports_windows() {
  std::vector<std::string> ports;
  return ports;
}
#endif

#if defined(__APPLE__)
static std::vector<std::string> list_serial_ports_mac() {
  std::vector<std::string> ports;
  for (const auto &entry : std::filesystem::directory_iterator("/dev")) {
    const auto name = entry.path().filename().string();
    if (name.rfind("cu.", 0) == 0 || name.rfind("tty.", 0) == 0) {
      ports.push_back("/dev/" + name);
    }
  }
  return ports;
}
#endif

#if defined(__linux__)
static std::vector<std::string> list_serial_ports_linux() {
  std::vector<std::string> ports;
  for (const auto &entry : std::filesystem::directory_iterator("/dev")) {
    const auto name = entry.path().filename().string();
    if (name.rfind("ttyUSB", 0) == 0 || name.rfind("ttyACM", 0) == 0 ||
        name.rfind("ttyS", 0) == 0) {
      ports.push_back("/dev/" + name);
    }
  }
  return ports;
}
#endif

std::vector<std::string> list_serial_ports() {
#if defined(_WIN32)
  return list_serial_ports_windows();
#elif defined(__APPLE__)
  return list_serial_ports_mac();
#elif defined(__linux__)
  return list_serial_ports_linux();
#else
  return {};
#endif
}

} // namespace serial
