#include "serial_inputs.hpp"
#include <string>
#include <vector>

#if defined(_WIN32)
// Windows-only includes
#include <devguid.h>
#include <regstr.h>
#include <setupapi.h>
#include <windows.h>
#pragma comment(lib, "setupapi.lib")
#elif defined(__APPLE__) || defined(__linux__)
#include <filesystem>
#endif

#if defined(_WIN32)
std::vector<std::string> listSerialPortsWindows() {
  std::vector<std::string> ports;
  // ... your SetupAPI code ...
  return ports;
}
#endif

#if defined(__APPLE__)
std::vector<std::string> listSerialPortsMac() {
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
std::vector<std::string> listSerialPortsLinux() {
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

std::vector<std::string> listSerialPorts() {
#if defined(_WIN32)
  return listSerialPortsWindows();
#elif defined(__APPLE__)
  return listSerialPortsMac();
#elif defined(__linux__)
  return listSerialPortsLinux();
#else
  return {};
#endif
}
