#include "can_sniffer.hpp"
#include "CAN_sniffer/libs/candbc_parser.hpp"
#include "components/components.hpp"
#include "app_main.hpp"
#include "imgui.h"
#include "implot.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

/*
 * Shares similar viewport as normal Debug Plotter with some additions:
 * 1. adds a console to print the values in text
 * 2. uses a bar plot or maybe a line graph for specific signals
 * 3. has a section where user has to provide a DBC file for decryption
 */


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
// Helper: render a bar plot for a list of signals with hover tooltips
static void RenderSignalBarPlot(const char *plotTitle,
                                const std::vector<std::pair<uint32_t, const CANDBC_PARSER::SignalValue *>> &signals) {
  if (signals.empty())
    return;

  // Collect tick labels and positions
  std::vector<double> positions;
  std::vector<const char *> labels;
  std::vector<double> values;
  for (int i = 0; i < (int)signals.size(); i++) {
    positions.push_back(i);
    labels.push_back(signals[i].second->name.c_str());
    values.push_back(signals[i].second->physicalValue);
  }

  static const double barWidth = 0.6;
  float plotHeight = 120.0f;
  if (ImPlot::BeginPlot(plotTitle, ImVec2(-1, plotHeight))) {
    ImPlot::SetupAxes(nullptr, "Value");
    ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks);
    ImPlot::SetupAxisLimits(ImAxis_X1, -0.5, (double)signals.size() - 0.5, ImPlotCond_Always);
    // Auto-fit Y axis to data range with padding
    double yMin = 0, yMax = 0;
    for (double v : values) {
      if (v < yMin) yMin = v;
      if (v > yMax) yMax = v;
    }
    double yPad = (yMax - yMin) * 0.1;
    if (yPad < 1.0) yPad = 1.0;
    ImPlot::SetupAxisLimits(ImAxis_Y1, yMin - yPad, yMax + yPad, ImPlotCond_Always);

    ImPlot::PlotBars("Signals", positions.data(), values.data(),
                     (int)signals.size(), barWidth);

    // Hover tooltip: determine which bar the mouse is over
    if (ImPlot::IsPlotHovered()) {
      ImPlotPoint mouse = ImPlot::GetPlotMousePos();
      int idx = (int)(mouse.x + 0.5);
      if (idx >= 0 && idx < (int)signals.size()) {
        double barLeft = positions[idx] - barWidth / 2.0;
        double barRight = positions[idx] + barWidth / 2.0;
        if (mouse.x >= barLeft && mouse.x <= barRight) {
          ImGui::BeginTooltip();
          ImGui::Text("Signal: %s", signals[idx].second->name.c_str());
          ImGui::Text("Value:  %.2f %s", signals[idx].second->physicalValue,
                       signals[idx].second->unit.c_str());
          ImGui::Text("Raw:    0x%X", signals[idx].second->rawValue);
          ImGui::Text("Msg ID: 0x%X", signals[idx].first);
          ImGui::EndTooltip();
        }
      }
    }
    ImPlot::EndPlot();
  }
}

void Bar_plot() {
  // Partition signals by their individual multiplexed flag, not the message-level flag.
  // This ensures non-multiplexed signals inside mixed messages still appear correctly.
  struct MuxMsgSignals {
    const CANDBC_PARSER::MessageData *msg;
    std::vector<std::pair<uint32_t, const CANDBC_PARSER::SignalValue *>> sigs;
  };
  std::map<uint32_t, MuxMsgSignals> multiplexed_msgs;
  std::vector<std::pair<uint32_t, const CANDBC_PARSER::SignalValue *>> non_mux_signals;

  for (const auto &entry : CANDBC_PARSER::signal_store) {
    const auto &msgData = entry.second;
    for (const auto &sig : msgData.signals) {
      if (sig.second.isMultiplexed) {
        auto &mux = multiplexed_msgs[msgData.id];
        mux.msg = &msgData;
        mux.sigs.push_back({msgData.id, &sig.second});
      } else {
        non_mux_signals.push_back({msgData.id, &sig.second});
      }
    }
  }

  // Render individual plots for each multiplexed message (sorted by signal name)
  for (auto &[id, muxMsg] : multiplexed_msgs) {
    std::sort(muxMsg.sigs.begin(), muxMsg.sigs.end(),
              [](const auto &a, const auto &b) {
                return a.second->name < b.second->name;
              });
    char title[128];
    snprintf(title, sizeof(title), "%s (0x%X)", muxMsg.msg->name.c_str(),
             muxMsg.msg->id);
    RenderSignalBarPlot(title, muxMsg.sigs);
  }

  // Render one combined plot for all non-multiplexed signals
  RenderSignalBarPlot("Non-Multiplexed Signals", non_mux_signals);
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
    log.AddLog("%s\n", line.c_str());
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
 * the specific signal value. Iterates the DBC messages directly so all signals
 * are shown even without live CAN traffic.
 * --------------------------------------------------------
 */
void CAN_IG::CAN_IG::gen_tables() {
  const auto &messages = CAN_MODE_WINDOW::parser.getMessages();
  if (messages.empty()) {
    ImGui::TextDisabled("No DBC messages loaded...");
    return;
  }

  // Show selected signal info at the top
  if (hasSelection) {
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f),
                       "Selected: %s  (Msg 0x%X)",
                       selectedSignalName.c_str(), selectedMsgId);
    ImGui::Separator();
  }

  for (const auto &[msgId, msg] : messages) {
    // Tree node per message: "MessageName (0xID) - N signals"
    char header[128];
    snprintf(header, sizeof(header), "%s (0x%X) - %zu signals",
             msg.name.c_str(), msg.id,
             msg.signals.size());

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
    if (!ImGui::TreeNodeEx(header, nodeFlags))
      continue;

    // Use a unique table ID per message to avoid ImGui ID collisions
    char tableId[64];
    snprintf(tableId, sizeof(tableId), "##signals_%X", msg.id);

    if (ImGui::BeginTable(tableId, 6,
                          ImGuiTableFlags_Resizable |
                              ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_Borders |
                              ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("Signal",  ImGuiTableColumnFlags_None, 3.0f);
      ImGui::TableSetupColumn("Unit",    ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableSetupColumn("Min",     ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableSetupColumn("Max",     ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableSetupColumn("Factor",  ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableSetupColumn("Offset",  ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableHeadersRow();

      // Collect signals and sort with natural numeric ordering
      std::vector<std::pair<std::string, const Vector::DBC::Signal *>> sortedSigs;
      for (const auto &[sName, s] : msg.signals)
        sortedSigs.push_back({sName, &s});
      std::sort(sortedSigs.begin(), sortedSigs.end(),
                [](const auto &a, const auto &b) {
                  const auto &sa = a.first;
                  const auto &sb = b.first;
                  size_t ia = 0, ib = 0;
                  while (ia < sa.size() && ib < sb.size()) {
                    if (std::isdigit(sa[ia]) && std::isdigit(sb[ib])) {
                      size_t na = ia, nb = ib;
                      while (na < sa.size() && std::isdigit(sa[na])) na++;
                      while (nb < sb.size() && std::isdigit(sb[nb])) nb++;
                      size_t lenA = na - ia, lenB = nb - ib;
                      constexpr size_t maxDigits = 18;
                      if (lenA > maxDigits || lenB > maxDigits) {
                        // Too long for stoul: compare by length then lexicographically
                        if (lenA != lenB) return lenA < lenB;
                        int cmp = sa.compare(ia, lenA, sb, ib, lenB);
                        if (cmp != 0) return cmp < 0;
                      } else {
                        unsigned long va = std::stoul(sa.substr(ia, lenA));
                        unsigned long vb = std::stoul(sb.substr(ib, lenB));
                        if (va != vb) return va < vb;
                      }
                      ia = na; ib = nb;
                    } else {
                      if (sa[ia] != sb[ib]) return sa[ia] < sb[ib];
                      ia++; ib++;
                    }
                  }
                  return sa.size() < sb.size();
                });

      for (const auto &[sigName, sigPtr] : sortedSigs) {
        const auto &sig = *sigPtr;
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        bool isSelected = hasSelection && selectedMsgId == msgId &&
                          selectedSignalName == sigName;

        if (ImGui::Selectable(sig.name.c_str(), isSelected,
                              ImGuiSelectableFlags_SpanAllColumns)) {
          hasSelection = true;
          selectedMsgId = msgId;
          selectedSignalName = sigName;
          // Reset input value to signal minimum when selecting a new signal
          inputValue = sig.minimum;
        }

        ImGui::TableNextColumn();
        ImGui::Text("%s", sig.unit.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", sig.minimum);
        ImGui::TableNextColumn();
        ImGui::Text("%.2f", sig.maximum);
        ImGui::TableNextColumn();
        ImGui::Text("%.4g", sig.factor);
        ImGui::TableNextColumn();
        ImGui::Text("%.4g", sig.offset);
      }
      ImGui::EndTable();
    }
    ImGui::TreePop();
  }
}

CAN_IG::CAN_IG() {}

void CAN_IG::CAN_IG::RenderUI() {
  if (!CAN_MODE_WINDOW::parser.is_loaded())
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
  ImGui::Begin("CAN IG Tables");

  // --- Signal tables (scrollable region) ---
  float sendPanelHeight = hasSelection ? 120.0f : 0.0f;
  ImGui::BeginChild("TablesScroll", ImVec2(0, -sendPanelHeight),
                    ImGuiChildFlags_None,
                    ImGuiWindowFlags_AlwaysVerticalScrollbar);
  this->gen_tables();
  ImGui::EndChild();

  // --- Send panel (only when a signal is selected) ---
  if (hasSelection) {
    ImGui::Separator();

    const auto &messages = CAN_MODE_WINDOW::parser.getMessages();
    const Vector::DBC::Signal *selSignal = nullptr;
    const Vector::DBC::Message *selMsg = nullptr;

    {
      auto msgIt = messages.find(selectedMsgId);
      if (msgIt != messages.end()) {
        selMsg = &msgIt->second;
        auto sigIt = msgIt->second.signals.find(selectedSignalName);
        if (sigIt != msgIt->second.signals.end()) {
          selSignal = &sigIt->second;
        }
      }
    }

    if (selSignal && selMsg) {
      ImGui::Text("Signal: %s  |  Message: %s (0x%X)  |  Range: [%.2f .. %.2f] %s",
                  selSignal->name.c_str(), selMsg->name.c_str(), selMsg->id,
                  selSignal->minimum, selSignal->maximum,
                  selSignal->unit.c_str());

      ImGui::SetNextItemWidth(200.0f);
      ImGui::InputDouble("Physical Value", &inputValue, 0.1, 1.0, "%.4f");

      // Clamp to signal min/max (unless DBC has 0/0 which means unconstrained)
      if (selSignal->minimum != 0.0 || selSignal->maximum != 0.0) {
        if (inputValue < selSignal->minimum) inputValue = selSignal->minimum;
        if (inputValue > selSignal->maximum) inputValue = selSignal->maximum;
      }

      ImGui::SameLine();
      if (ImGui::Button("Send")) {
        // 1. Convert physical value to raw
        double rawValue = selSignal->physicalToRawValue(inputValue);

        // 2. Validate rawValue before casting (NaN / out-of-range for int64_t)
        constexpr double kInt64Min = static_cast<double>(std::numeric_limits<int64_t>::min());
        constexpr double kInt64Max = static_cast<double>(std::numeric_limits<int64_t>::max());
        if (std::isnan(rawValue) || rawValue < kInt64Min || rawValue > kInt64Max) {
          ImGui::SameLine();
          ImGui::TextColored(ImVec4(1, 0, 0, 1), "Raw value out of range!");
        } else {
          // 3. Initialize buffer from last-known frame data to preserve other signals
          std::vector<uint8_t> buffer(selMsg->size, 0);
          auto storeIt = CANDBC_PARSER::signal_store.find(selectedMsgId);
          if (storeIt != CANDBC_PARSER::signal_store.end() &&
              !storeIt->second.lastFrameData.empty()) {
            buffer = storeIt->second.lastFrameData;
            buffer.resize(selMsg->size, 0);
          }

          // 4. Encode raw value into the buffer using a local Signal copy
          uint64_t rawU64 = static_cast<uint64_t>(static_cast<int64_t>(rawValue));
          Vector::DBC::Signal localSignal = *selSignal;
          localSignal.encode(buffer, rawU64);

          // 5. Convert buffer to hex string
          std::string hexData;
          hexData.reserve(buffer.size() * 2);
          for (uint8_t byte : buffer) {
            char hex[3];
            snprintf(hex, sizeof(hex), "%02X", byte);
            hexData += hex;
          }

          // 6. Build JSON and send
          nlohmann::json j;
          j["id"] = selMsg->id;
          j["data"] = hexData;
          std::string jsonStr = j.dump() + "\n";
          MyApp::serialReader.Send(jsonStr);
        }
      }
    } else {
      ImGui::TextDisabled("Selected signal no longer found in DBC.");
      if (ImGui::Button("Clear Selection")) {
        hasSelection = false;
      }
    }
  }

  ImGui::End();
}

} // namespace CAN_IG
