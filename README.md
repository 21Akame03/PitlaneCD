# Pitlane_ControlDesk
[![Build](https://github.com/21Akame03/Pitlane_ControlDesk/actions/workflows/build.yml/badge.svg)](https://github.com/21Akame03/Pitlane_ControlDesk/actions/workflows/build.yml)

ControlDesk-like development UI for **pitlane_rs**.

Pitlane_ControlDesk is used during development of the **2027 E-Traxx ECUs** to visualize and inspect data streamed over USB/Serial (and similar links). It provides an Arduino Serial Plotter–style experience, plus the kind of signal/parameter dashboard workflow you’d expect from tools like dSPACE ControlDesk.

Compatible with **ESP32** and **STM32** (and generally anything that can stream structured telemetry).

---

## Supported Platforms

| OS | Architecture | Status |
|----|-------------|--------|
| macOS | ARM (Apple Silicon) | Supported |
| macOS | x86_64 | Supported |
| Linux | x86_64 | Supported |
| Linux | ARM64 | Supported |
| Windows | x86_64 | Supported |
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

### Windows

```bash
# Using Visual Studio Developer Command Prompt or MSYS2
cmake -B build
cmake --build build

# Run
.\build\Release\glfw_opengl3.exe
```

> **Note:** GLFW and all other dependencies are fetched and built automatically. No manual dependency installation is required.

### Cross-compiling (macOS Universal Binary)

```bash
cmake -B build -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build
```

---

Maintainers: @Ubdhoot Ashitosh
