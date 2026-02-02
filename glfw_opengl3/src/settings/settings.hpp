#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>

namespace SETTINGS {

// Saves the state of the checkboxes and the value represented by the checkboxes
typedef struct {
  std::string VariableName;
  bool isChecked;
  double value;
} VariableCheckbox;

void RenderUI();

} // namespace SETTINGS

#endif
