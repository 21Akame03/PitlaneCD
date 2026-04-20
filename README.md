# Pitlane_ControlDesk

<p align="center">
  <img src="/assets/pitlane_2.png" alt="E-Traxx Cover" width="100%">
</p>

ControlDesk-like development UI for **pitlane_rs**.

### Relay (ESP32S3 based)
[ESP32S3 Based Relay for Pitlane](https://github.com/21Akame03/Pitlane_relay)

Pitlane_ControlDesk is used during development of the **2027 E-Traxx ECUs** to visualize and inspect data streamed over USB/Serial (and similar links). It provides an Arduino Serial Plotter–style experience, plus the kind of signal/parameter dashboard workflow you’d expect from tools like dSPACE ControlDesk.

Compatible with **ESP32** and **STM32** (and generally anything that can stream structured telemetry).

---

## Supported Platforms

| OS | Architecture | Status |
|----|-------------|--------|
| macOS | ARM (Apple Silicon) | Supported |
| macOS | x86_64 | Supported |
| Linux | x86_64 | *Supported |
| Linux | ARM64 | Not Supported |
| Windows | x86_64 | Not Supported |
| Windows | ARM64 | Experimental (GLFW 3.4+) |

## Building

All dependencies (GLFW, ImGui, ImPlot, ImGuiFileDialog, Vector DBC, nlohmann/json) are fetched automatically via CMake FetchContent. GLFW will use a system install if available, otherwise it is built from source.

### Prerequisites

- CMake 3.15+
- C++17 compatible compiler
- OpenGL development libraries
- Flex and Bison

### macOS

```bash
# Install build tools
brew install cmake flex bison

# Configure and build
cmake -B build
cmake --build build

# Run
./build/glfw_opengl3
```

> **Note:** GLFW is fetched automatically if not installed. To use a system install: `brew install glfw`

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install cmake build-essential libgl-dev flex bison

# Configure and build
cmake -B build
cmake --build build

# Run
./build/glfw_opengl3
```

> **Note:** On Linux, GLFW is fetched from source if `libglfw3-dev` is not installed. Installing it via your package manager (`sudo apt install libglfw3-dev`) avoids the extra build time.

### Windows (native)

```bash
# Using Visual Studio Developer Command Prompt or MSYS2
cmake -B build
cmake --build build

# Run
.\build\Release\glfw_opengl3.exe
```

> **Note:** GLFW and all other dependencies are fetched and built automatically. No manual dependency installation is required.

### Windows (cross-compile from macOS)

You can build a Windows `.exe` from macOS using the MinGW-w64 cross-compiler.

```bash
# Install MinGW-w64 and Boost (header-only, needed for boost::asio)
brew install mingw-w64 boost

# Configure with the provided MinGW toolchain file
cmake -B build_win64 \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw64.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_DISABLE_FIND_PACKAGE_glfw3=ON \
  -DFLEX_EXECUTABLE=/opt/homebrew/opt/flex/bin/flex \
  -DFLEX_INCLUDE_DIR=/opt/homebrew/opt/flex/include \
  -DCMAKE_CXX_FLAGS="-I/opt/homebrew/include"

# Build
cmake --build build_win64 --config Release -j$(sysctl -n hw.ncpu)
```

The output binary is at `build_win64/glfw_opengl3.exe` (PE32+ x86-64).

> **Note:** `-DCMAKE_DISABLE_FIND_PACKAGE_glfw3=ON` forces GLFW to be built from source instead of picking up the macOS system install. The Boost include path is needed because `boost::asio` (used for serial communication) is header-only but not fetched via CMake.

### Cross-compiling (macOS Universal Binary)

```bash
cmake -B build -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build
```

---

Maintainers: @Ubdhoot Ashitosh
