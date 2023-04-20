#include "helpers.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

#include "../constants.h"

namespace CG {

inline bool is_one_digit(auto value) {
  return 0 <= value && value < 10;
}

std::stringstream& output_grid(
    std::stringstream& ss,
    int width,
    int height,
    const std::vector<int>& squares) {
  for (auto y = 0; y < height; ++y) {
    for (auto x = 0; x < width; ++x) {
      const auto value = squares[x + width * y];
      ss << (value == INT::INFTY || value == -1 ? "X " : std::to_string(value))
         << (is_one_digit(value) ? "  " : " ");
    }
    ss << '\n';
  }
  return ss;
}

std::string error_msg_distance_fields(
    int width,
    int height,
    const std::vector<int>& expected,
    const std::vector<int>& actual) {
  std::stringstream ss;
  ss << "Expected\n";
  output_grid(ss, width, height, expected);

  ss << "\nBut got\n";
  output_grid(ss, width, height, actual);

  return ss.str();
}

} // namespace CG
