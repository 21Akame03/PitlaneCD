#include "serial/serial_reader.hpp"
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#elif defined(__APPLE__) || defined(__linux__)
#include <filesystem>
#endif

namespace serial {

#if defined(_WIN32)
// Enumerate COM ports using QueryDosDeviceA.
// Only requires <windows.h> — no setupapi, no registry walk.
// Finds every COMn the kernel has a device object for: USB-serial (CDC ACM),
// Bluetooth SPP virtual ports, and hardware UARTs.
static std::vector<std::string> list_serial_ports_windows() {
  std::vector<std::string> ports;

  // QueryDosDevice with NULL lpDeviceName fills the buffer with all current
  // DOS device names as a double-null-terminated list of null-terminated strings.
  constexpr DWORD kBufSize = 65536;
  std::vector<char> buf(kBufSize, '\0');
  if (QueryDosDeviceA(nullptr, buf.data(), kBufSize) == 0)
    return ports;

  for (const char* p = buf.data(); *p != '\0'; p += std::strlen(p) + 1) {
    std::string name(p);
    // Keep only names matching COMn (case-insensitive, any number of digits).
    if (name.size() >= 4 &&
        (name[0] == 'C' || name[0] == 'c') &&
        (name[1] == 'O' || name[1] == 'o') &&
        (name[2] == 'M' || name[2] == 'm') &&
        std::isdigit(static_cast<unsigned char>(name[3])))
    {
      // Boost.Asio requires the \\.\COMn form for all ports on Windows
      // (mandatory for COM10+, harmless for COM1-COM9).
      ports.push_back("\\\\.\\" + name);
    }
  }

  // Sort numerically: COM1, COM2, ..., COM10, COM11, ...
  // The prefix "\\.\COM" is 7 characters, so skip 7 to reach the number.
  std::sort(ports.begin(), ports.end(), [](const std::string& a, const std::string& b) {
    return std::atoi(a.c_str() + 7) < std::atoi(b.c_str() + 7);
  });

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
