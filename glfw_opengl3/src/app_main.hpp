
#include "imgui.h"
#include "settings/settings.hpp"
#include <string>
#include <vector>

namespace MyApp {
void RenderUI();

// /*
//  * Purpose: Saves the backend states such as Serial connection status or
//  number
//  * of active variables
//  */
// typedef struct {
//   bool SerialConnected;
//   std::vector<std::string> com_ports;
// } Settings_state_t;

// is used to save the mode of the program
typedef enum {
  NONE,
  CAN_SNIFFER,
  DEBUG,
  TELEMETRY,
} Mode_t;

extern Mode_t mode;

// To avoid ODR violations, do not define this anywhere else
extern std::vector<SETTINGS::VariableCheckbox> variables;

} // namespace MyApp
