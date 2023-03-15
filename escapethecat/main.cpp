#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <iterator>
#include <functional>
#include <unordered_map>
#include <vector>

#include "catandmouse.h"

using namespace EscapeTheCat;

static const double TICK_ANGLE = std::asin(1 / std::sqrt(101));

inline double dot(const std::complex<double>& a, const std::complex<double>& b) {
  return a.real() * b.real() + a.imag() * b.imag();
}

inline std::ostream& operator<<(std::ostream& os, const std::complex<double>& pos) {
  return os << static_cast<int>(pos.real()) << ' ' << static_cast<int>(pos.imag());
}

int main() {
  bool stage1 = true;

  CatAndMouse cm = initialize(std::cin);

  // Generate the targets for a mouse sitting in the center.
  const std::vector<std::complex<double>> center_ticks = [](){
    std::vector<std::complex<double>> ret(1, {MOUSE_SPEED, 0});
    double arg = 0.0;
    const std::complex<double> tick_rot = std::polar(1.0, TICK_ANGLE);
    while (arg < 2 * M_PI - TICK_ANGLE) {
      ret.push_back(ret.back() * tick_rot);
      arg += TICK_ANGLE;
    }
    return ret;
  }();

  std::vector<std::complex<double>> mouse_ticks = center_ticks;
  std::vector<std::complex<double>> ticks;
  std::vector<std::pair<decltype(ticks)::const_iterator, double>> scores;

  while (true) {
    std::cin >> cm; std::cin.ignore();

    // Terminate stage 1 upon reaching the center
    if (abs(cm.mouse()) < MOUSE_SPEED / 2) {
      stage1 = false;
      std::cerr << "Reached center" << std::endl;
    }

    // Go towards the center during stage 1
    if (stage1) {
      std::cout << "0 0 Towards Center!" << std::endl;
      continue;
    }

    // Update the targets to lie around the mouse
    std::transform(center_ticks.begin(), center_ticks.end(), mouse_ticks.begin(),
                   [&m=cm.mouse()](const auto& tick) {
                     return m + tick;
                   });

    // Only consider targets towards boundary
    ticks.clear();
    std::copy_if(mouse_ticks.begin(), mouse_ticks.end(), std::back_inserter(ticks),
                 [&](const auto& tick) {
                   return dot(cm.cat() - cm.mouse(), tick - cm.mouse()) < 0;
                 });

    // Compute each target's score
    scores.clear();
    std::transform(ticks.begin(), ticks.end(), std::back_inserter(scores),
                   [&cm, it=ticks.cbegin()](const auto& tick) mutable {
                     cm.step(tick);
                     double score = cm.score();
                     cm.undo();
                     return std::make_pair(it++, score);
                   });

    // Pick the target with the highest score
    std::cout << *std::max_element(scores.begin(), scores.end(),
                                  [](const auto& a, const auto& b) {
                                    return a.second < b.second;
                                  })->first
              << std::endl;
  }
}
