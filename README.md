# Pitlane_ControlDesk
[![Build](https://github.com/21Akame03/Pitlane_ControlDesk/actions/workflows/build.yml/badge.svg)](https://github.com/21Akame03/Pitlane_ControlDesk/actions/workflows/build.yml)

ControlDesk-like development UI for **pitlane_rs**.

Pitlane_ControlDesk is used during development of the **2027 E-Traxx ECUs** to visualize and inspect data streamed over USB/Serial (and similar links). It provides an Arduino Serial Plotter–style experience, plus the kind of signal/parameter dashboard workflow you’d expect from tools like dSPACE ControlDesk.

Compatible with **ESP32** and **STM32** (and generally anything that can stream structured telemetry).

---

## Building

### Prerequisites

- CMake 3.15+
- Ninja build system
- C++17 compatible compiler
- GLFW3 (`brew install glfw` on macOS)
- Flex and Bison (`brew install flex bison` on macOS)

### macOS

```bash
# Configure and build
cmake -B build -G Ninja -DFLEX_INCLUDE_DIR=/Library/Developer/CommandLineTools/usr/include
cmake --build build

# Run
./build/glfw_opengl3
```

> **Note:** The `FLEX_INCLUDE_DIR` flag is required because CMake's FindFLEX module on macOS finds the flex executable but not the `FlexLexer.h` header. This header is typically located at `/Library/Developer/CommandLineTools/usr/include`. If you installed flex via Homebrew, the path might be `/opt/homebrew/Cellar/flex/<version>/include` instead.

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install cmake ninja-build libglfw3-dev flex bison

# Configure and build
cmake -B build -G Ninja
cmake --build build

# Run
./build/glfw_opengl3
```

### Windows

Use the provided `build_win32.bat` script or configure with CMake:

```bash
cmake -B build -G Ninja
cmake --build build
```

---

Maintainers: @Ubdhoot Ashitosh
