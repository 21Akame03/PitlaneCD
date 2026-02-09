#ifndef CAN_SNIFFER_HPP
#define CAN_SNIFFER_HPP

#include "CAN_sniffer/libs/candbc_parser.hpp"
#include "imgui.h"
#include <cstdarg>
#include <string>

struct AppLog {
  ImGuiTextBuffer Buf;
  ImGuiTextFilter Filter;
  ImVector<int> LineOffsets;
  bool AutoScroll;

  AppLog() {
    AutoScroll = true;
    Clear();
  }

  void Clear() {
    Buf.clear();
    LineOffsets.clear();
    LineOffsets.push_back(0);
  }

  void AddLog(const char *fmt, ...) IM_FMTARGS(2) {
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
      if (Buf[old_size] == '\n')
        LineOffsets.push_back(old_size + 1);
  }

  void Draw(const char *title, bool *p_open = NULL);
};

namespace CAN_MODE_WINDOW {

extern AppLog log;

class Sniffer_window {
public:
  Sniffer_window();
  void RenderUI();
};

extern CANDBC_PARSER::DBCParser parser;

} // namespace CAN_MODE_WINDOW

namespace CAN_IG {

class CAN_IG {
public:
  CAN_IG();
  void RenderUI();

private:
  static void gen_tables();
};
} // namespace CAN_IG
#endif
