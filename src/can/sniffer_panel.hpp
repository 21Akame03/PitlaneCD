#ifndef SNIFFER_PANEL_HPP
#define SNIFFER_PANEL_HPP

#include "can/dbc_parser.hpp"
#include "serial/serial_reader.hpp"
#include "ui/app_log.hpp"

namespace can {

class SnifferPanel {
public:
  SnifferPanel(DbcParser &parser, AppLog &log, serial::SerialReader &reader);
  void render_ui();

private:
  void render_bar_plot();
  static void render_signal_bar_plot(
      const char *title,
      const std::vector<std::pair<uint32_t, const SignalValue *>> &signals);

  DbcParser &parser_;
  AppLog &log_;
  serial::SerialReader &reader_;
};

} // namespace can

#endif // SNIFFER_PANEL_HPP
