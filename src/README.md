# src/ — PitlaneCD Source Structure

## Architecture

A single `App` struct (defined in `app.hpp`) owns all subsystems. Dependencies are passed via constructor references — no globals.

```
App
├── serial::SerialReader    — async serial I/O (Boost ASIO, background thread)
├── can::DbcParser          — DBC file loading + CAN frame decoding
├── AppLog                  — shared ImGui log widget
├── can::SnifferPanel       — CAN message bar plots + log (refs: parser, log, reader)
├── can::IgPanel            — CAN signal tables + send panel (refs: parser, reader)
├── debug::DebugPanel       — telemetry log + real-time plotter (refs: reader)
└── ui::SettingsPanel       — serial port selector, DBC browser (refs: reader, parser, mode)
```

## Directory Layout

```
src/
├── app.hpp/cpp                  App struct, docking layout, mode selector
├── serial/
│   ├── serial_reader.hpp/cpp    SerialReader class (thread-safe, Boost ASIO)
│   └── port_discovery.cpp       Platform-specific serial port enumeration
├── can/
│   ├── dbc_parser.hpp/cpp       DbcParser, SignalValue, MessageData
│   ├── sniffer_panel.hpp/cpp    SnifferPanel — bar plots + log rendering
│   └── ig_panel.hpp/cpp         IgPanel — signal tables + CAN frame sending
├── debug/
│   ├── log_parser.hpp/cpp       LogEntry, PlotSeries, AppState line parser
│   └── debug_panel.hpp/cpp      DebugPanel — colored log + ImPlot time series
└── ui/
    ├── app_log.hpp/cpp          AppLog — filterable, auto-scrolling text log
    └── settings_panel.hpp/cpp   SettingsPanel — connection + DBC file UI
```

## Data Flow

```
Serial Port (USB/UART)
    │
    ▼
SerialReader (background thread, Boost ASIO)
    │  PollRxBuffer() — thread-safe swap
    ▼
Main Thread (each frame)
    ├─► DebugPanel
    │     Feed lines → LogParser → LogEntry + PlotSeries
    │     Render colored log + ImPlot time series
    │
    └─► SnifferPanel
          Feed lines → DbcParser.parse_frame() → signal_store
          Render bar plots + AppLog
```

## Modes

The app starts with a mode selector popup:

| Mode | Panels shown |
|------|-------------|
| **Debug** | DebugPanel (log + plotter) |
| **CAN Sniffer** | SnifferPanel (bar plots + log) + IgPanel (signal tables + send) |

SettingsPanel is always visible in both modes.

## Adding New Features

### New UI panel (e.g. a signal dashboard)

1. Create `src/<namespace>/<panel_name>.hpp/cpp` (e.g. `src/can/dashboard_panel.hpp`).
2. Define a class with a `render_ui()` method. Accept dependencies as constructor references:
   ```cpp
   namespace can {
   class DashboardPanel {
   public:
     DashboardPanel(DbcParser &parser, serial::SerialReader &reader);
     void render_ui();
   private:
     DbcParser &parser_;
     serial::SerialReader &reader_;
   };
   }
   ```
3. Add the panel as a member of `App` in `app.hpp`, initialize it in the `App` constructor.
4. Call `panel.render_ui()` inside `App::RenderUI()` under the appropriate mode `switch` case.
5. If the panel needs a default dock position, add an `ImGui::DockBuilderDockWindow()` call in `App::setup_default_docking_layout()`.

### New app mode (e.g. Telemetry)

1. Add the mode to `ui::AppMode` enum in `ui/settings_panel.hpp`.
2. Add a button for it in `App::mode_selector()` in `app.cpp`.
3. Add a `case` in the `switch` block inside `App::RenderUI()`.
4. Create the panel class(es) for the mode following the steps above.

### New data parser or protocol

1. Create the parser in its own file under the relevant namespace (e.g. `src/can/`, `src/debug/`).
2. Keep parsers **free of UI dependencies** — no ImGui includes. Use callbacks or return values to communicate results.
3. If the parser needs to log, accept a `std::function<void(const std::string&)>` callback (see `DbcParser` for the pattern).

### New shared widget

1. Place it in `src/ui/` (e.g. `src/ui/status_bar.hpp/cpp`).
2. Widgets should be self-contained — accept data via constructor or method parameters, not globals.

### Conventions

- **Namespaces**: lowercase (`serial`, `can`, `debug`, `ui`).
- **Classes**: `PascalCase` (`SnifferPanel`, `DbcParser`).
- **Methods/files**: `snake_case` (`render_ui`, `sniffer_panel.cpp`).
- **No globals** — everything flows through `App` and constructor-injected references.
- **Parsers never import UI code** — use callbacks for logging.

## Key Design Decisions

- **Constructor injection** over globals — each subsystem declares exactly what it needs.
- **DbcParser uses a log callback** (`std::function<void(const std::string&)>`) so it has zero UI dependencies.
- **signal_store is private** inside `DbcParser`, exposed via `const` getter. Parser writes, sniffer reads.
- **SerialReader is thread-safe** — mutex-protected deque with swap-based polling. UI never blocks on I/O.
