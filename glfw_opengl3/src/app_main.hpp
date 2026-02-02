
#include "imgui.h"
#include "settings/settings.hpp"
#include <string>
#include <vector>

namespace MyApp {
void RenderUI();

namespace States {
/*
 * Purpose: Saves the backend states such as Serial connection status or number
 * of active variables
 */
typedef struct {
  bool SerialConnected;
  std::vector<std::string> com_ports;
} Settings_state_t;

} // namespace States

// To avoid ODR violations, do not define this anywhere else
extern std::vector<SETTINGS::VariableCheckbox> variables;

} // namespace MyApp
