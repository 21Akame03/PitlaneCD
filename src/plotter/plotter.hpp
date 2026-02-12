#ifndef PLOTTER_HPP
#define PLOTTER_HPP

#include <vector>

namespace PLOTTER {

class Plotter_window {
public:
  Plotter_window();

  void Plot_data(std::vector<float> x, std::vector<float> y);
  void RenderUI();
};

} // namespace PLOTTER

#endif
