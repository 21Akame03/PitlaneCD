
#include "imgui.h"
#include "settings/settings.hpp"
#include <string>
#include <vector>

namespace MyApp {
void RenderUI();

// To avoid ODR violations, do not define this anywhere else
extern std::vector<SETTINGS::VariableCheckbox> variables;

} // namespace MyApp
