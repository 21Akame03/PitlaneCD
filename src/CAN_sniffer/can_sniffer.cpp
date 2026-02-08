#include "can_sniffer.hpp"
#include "CAN_sniffer/libs/candbc_parser.hpp"
#include "app_main.hpp"
#include "imgui.h"
#include "implot.h"
#include <deque>
#include <ostream>

/*
 * Shares similar viewport as normal Debug Plotter with some additions:
 * 1. adds a console to print the values in text
 * 2. uses a bar plot or maybe a line graph for specific signals
 * 3. has a section where user has to provide a DBC file for decryption
 */

void AppLog::Draw(const char *title, bool *p_open) {
  if (!ImGui::Begin(title, p_open)) {
    ImGui::End();
    return;
  }

  // Options menu
  if (ImGui::BeginPopup("Options")) {
    ImGui::Checkbox("Auto-scroll", &AutoScroll);
    ImGui::EndPopup();
  }

  // Main window
  if (ImGui::Button("Options"))
    ImGui::OpenPopup("Options");
  ImGui::SameLine();
  bool clear = ImGui::Button("Clear");
  ImGui::SameLine();
  bool copy = ImGui::Button("Copy");
  ImGui::SameLine();
  Filter.Draw("Filter", -100.0f);

  ImGui::Separator();

  if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None,
                        ImGuiWindowFlags_HorizontalScrollbar)) {
    if (clear)
      Clear();
    if (copy)
      ImGui::LogToClipboard();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    const char *buf = Buf.begin();
    const char *buf_end = Buf.end();
    if (Filter.IsActive()) {
      for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
        const char *line_start = buf + LineOffsets[line_no];
        const char *line_end = (line_no + 1 < LineOffsets.Size)
                                   ? (buf + LineOffsets[line_no + 1] - 1)
                                   : buf_end;
        if (Filter.PassFilter(line_start, line_end))
          ImGui::TextUnformatted(line_start, line_end);
      }
    } else {
      ImGuiListClipper clipper;
      clipper.Begin(LineOffsets.Size);
      while (clipper.Step()) {
        for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd;
             line_no++) {
          const char *line_start = buf + LineOffsets[line_no];
          const char *line_end = (line_no + 1 < LineOffsets.Size)
                                     ? (buf + LineOffsets[line_no + 1] - 1)
                                     : buf_end;
          ImGui::TextUnformatted(line_start, line_end);
        }
      }
      clipper.End();
    }
    ImGui::PopStyleVar();

    if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      ImGui::SetScrollHereY(1.0f);
  }
  ImGui::EndChild();
  ImGui::End();
}

namespace CAN_MODE_WINDOW {
CANDBC_PARSER::DBCParser parser;
AppLog log;

/*
 * --------------------------------------------------------
 * Purpose: Creates a Bar/histogram to show the current values of the decrypted
 * value Input: NONE
 * Output: NONE
 * --------------------------------------------------------
 */
void Bar_plot() {
  // Test data for visualisation test
  static ImS8 data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  if (ImPlot::BeginPlot("CAN Messages")) {
    ImPlot::PlotBars("Value", data, 10, 0, 7, 1);
    ImPlot::EndPlot();
  }
}

/*
 * --------------------------------------------------------
 * Purpose: Shows the log Window
 * value Input: NONE
 * Output: NONE
 * --------------------------------------------------------
 */

static void ShowAppLog() {

  // Reference: Example debug button for testing log entries
  // ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
  // ImGui::Begin("Log");
  // if (ImGui::SmallButton("[Debug] Add 5 entries")) {
  //   static int counter = 0;
  //   const char *categories[3] = {"info", "warn", "error"};
  //   const char *words[] = {"Bumfuzzled",    "Cattywampus",  "Snickersnee",
  //                          "Abibliophobia", "Absquatulate", "Nincompoop",
  //                          "Pauciloquent"};
  //   for (int n = 0; n < 5; n++) {
  //     const char *category = categories[counter % IM_COUNTOF(categories)];
  //     const char *word = words[counter % IM_COUNTOF(words)];
  //     log.AddLog(
  //         "[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
  //         ImGui::GetFrameCount(), category, ImGui::GetTime(), word);
  //     counter++;
  //   }
  // }
  // ImGui::End();

  log.Draw("Log");

  // pull data from buffer
  std::deque<std::string> data = MyApp::serialReader.PollRxBuffer();
  for (const auto &line : data) {
    std::cout << line << std::endl;
    // log.AddLog("%s\n", line.c_str());
    parser.parse_frame(line);
  }
}

/*
 * --------------------------------------------------------
 * Purpose: Defines Context in case context is not created
 * value Input: NONE
 * Output: NONE
 * --------------------------------------------------------
 */

Sniffer_window::Sniffer_window() {}

/*
 * --------------------------------------------------------
 * Purpose: Renders the UI for CAN_Sniffer
 * value Input: NONE
 * Output: NONE
 * --------------------------------------------------------
 */
void Sniffer_window::RenderUI() {
  ImGui::Begin("CAN Sniffer Window");

  // backend work
  // When dbc file has been selected and not yet loaded
  if (!parser.dbcfilepath_.empty() && !parser.is_loaded()) {
    // check validity of dbc file
    std::cout << "Loading DBC file" << parser.load_dbc(parser.dbcfilepath_)
              << std::endl;
  }

  Bar_plot();
  ShowAppLog();
  ImGui::End();
}

} // namespace CAN_MODE_WINDOW

namespace CAN_IG {

/*
 * --------------------------------------------------------
 * Purpose: For each message in DBC File, create a table and allow selection of
 * the specific signal value Input: NONE Output: NONE
 * --------------------------------------------------------
 */
void CAN_IG::CAN_IG::gen_tables() {
  static bool selected[10] = {};

  if (ImGui::BeginTable("split1", 3,
                        ImGuiTableFlags_Resizable |
                            ImGuiTableFlags_NoSavedSettings |
                            ImGuiTableFlags_Borders)) {
    for (int i = 0; i < 10; i++) {
      char label[32];
      sprintf(label, "Item %d", i);
      ImGui::TableNextColumn();
      ImGui::Selectable(label,
                        &selected[i]); // FIXME-TABLE: Selection overlap
    }
    ImGui::EndTable();
  }
  ImGui::Spacing();
  if (ImGui::BeginTable("split2", 3,
                        ImGuiTableFlags_Resizable |
                            ImGuiTableFlags_NoSavedSettings |
                            ImGuiTableFlags_Borders)) {
    for (int i = 0; i < 10; i++) {
      char label[32];
      sprintf(label, "Item %d", i);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Selectable(label, &selected[i],
                        ImGuiSelectableFlags_SpanAllColumns);
      ImGui::TableNextColumn();
      ImGui::Text("Some other contents");
      ImGui::TableNextColumn();
      ImGui::Text("123456");
    }
    ImGui::EndTable();
  }
};

/*
 * --------------------------------------------------------
 * Purpose: Defines Context in case context is not created
 * value Input: NONE
 * Output: NONE
 * --------------------------------------------------------
 */
CAN_IG::CAN_IG() {}

void CAN_IG::CAN_IG::RenderUI() {
  if (CAN_MODE_WINDOW::parser.is_loaded()) {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("CAN IG Tables");
    ImGui::BeginChild("TablesScroll", ImVec2(0, 0), ImGuiChildFlags_None,
                      ImGuiWindowFlags_AlwaysVerticalScrollbar);
    this->gen_tables();
    ImGui::EndChild();
    ImGui::End();
  }
}

} // namespace CAN_IG
