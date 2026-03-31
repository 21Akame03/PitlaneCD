#include "can/ig_panel.hpp"
#include "imgui.h"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <cstdio>
#include <vector>

namespace can {

IgPanel::IgPanel(DbcParser &parser, serial::SerialReader &reader)
    : parser_(parser), reader_(reader) {}

void IgPanel::gen_tables() {
  const auto *messages = parser_.getMessages();
  if (!messages || messages->empty()) {
    ImGui::TextDisabled("No DBC messages loaded...");
    return;
  }

  if (hasSelection) {
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f),
                       "Selected: %s  (Msg 0x%X)",
                       selectedSignalName.c_str(), selectedMsgId);
    ImGui::Separator();
  }

  for (const auto &[msgId, msg] : *messages) {
    char header[128];
    snprintf(header, sizeof(header), "%s (0x%X) - %zu signals",
             msg.name.c_str(), msg.id, msg.signals.size());

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
    if (!ImGui::TreeNodeEx(header, nodeFlags))
      continue;

    char tableId[64];
    snprintf(tableId, sizeof(tableId), "##signals_%X", msg.id);

    if (ImGui::BeginTable(tableId, 6,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_Borders |
                              ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("Signal", ImGuiTableColumnFlags_None, 3.0f);
      ImGui::TableSetupColumn("Unit", ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableSetupColumn("Min", ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableSetupColumn("Max", ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableSetupColumn("Factor", ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_None, 1.0f);
      ImGui::TableHeadersRow();

      std::vector<std::pair<std::string, const Vector::DBC::Signal *>>
          sortedSigs;
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
                      while (na < sa.size() && std::isdigit(sa[na]))
                        na++;
                      while (nb < sb.size() && std::isdigit(sb[nb]))
                        nb++;
                      unsigned long va = std::stoul(sa.substr(ia, na - ia));
                      unsigned long vb = std::stoul(sb.substr(ib, nb - ib));
                      if (va != vb)
                        return va < vb;
                      ia = na;
                      ib = nb;
                    } else {
                      if (sa[ia] != sb[ib])
                        return sa[ia] < sb[ib];
                      ia++;
                      ib++;
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

void IgPanel::render_ui() {
  if (!parser_.is_loaded())
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
  ImGui::Begin("CAN IG Tables");

  float sendPanelHeight = hasSelection ? 120.0f : 0.0f;
  ImGui::BeginChild("TablesScroll", ImVec2(0, -sendPanelHeight),
                    ImGuiChildFlags_None,
                    ImGuiWindowFlags_AlwaysVerticalScrollbar);
  gen_tables();
  ImGui::EndChild();

  if (hasSelection) {
    ImGui::Separator();

    const auto *messages = parser_.getMessages();
    const Vector::DBC::Signal *selSignal = nullptr;
    const Vector::DBC::Message *selMsg = nullptr;

    if (messages) {
      auto msgIt = messages->find(selectedMsgId);
      if (msgIt != messages->end()) {
        selMsg = &msgIt->second;
        auto sigIt = msgIt->second.signals.find(selectedSignalName);
        if (sigIt != msgIt->second.signals.end()) {
          selSignal = &sigIt->second;
        }
      }
    }

    if (selSignal && selMsg) {
      ImGui::Text(
          "Signal: %s  |  Message: %s (0x%X)  |  Range: [%.2f .. %.2f] %s",
          selSignal->name.c_str(), selMsg->name.c_str(), selMsg->id,
          selSignal->minimum, selSignal->maximum, selSignal->unit.c_str());

      ImGui::SetNextItemWidth(200.0f);
      ImGui::InputDouble("Physical Value", &inputValue, 0.1, 1.0, "%.4f");

      if (selSignal->minimum != 0.0 || selSignal->maximum != 0.0) {
        if (inputValue < selSignal->minimum)
          inputValue = selSignal->minimum;
        if (inputValue > selSignal->maximum)
          inputValue = selSignal->maximum;
      }

      ImGui::SameLine();
      if (ImGui::Button("Send")) {
        double rawValue = selSignal->physicalToRawValue(inputValue);
        std::vector<uint8_t> buffer(selMsg->size, 0);
        uint64_t rawU64 =
            static_cast<uint64_t>(static_cast<int64_t>(rawValue));
        const_cast<Vector::DBC::Signal *>(selSignal)->encode(buffer, rawU64);

        std::string hexData;
        hexData.reserve(buffer.size() * 2);
        for (uint8_t byte : buffer) {
          char hex[3];
          snprintf(hex, sizeof(hex), "%02X", byte);
          hexData += hex;
        }

        nlohmann::json j;
        j["id"] = selMsg->id;
        j["data"] = hexData;
        std::string jsonStr = j.dump() + "\n";
        reader_.Send(jsonStr);
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

} // namespace can
