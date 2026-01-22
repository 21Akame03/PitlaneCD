#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>
#include <vector>

namespace SETTINGS {

// Saves the state of the checkboxes
typedef struct {
  std::string VariableName;
  bool isChecked;
} VariableCheckbox;

void RenderUI();

} // namespace SETTINGS

#endif
