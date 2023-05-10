#include "helpers.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "../constants.h"

namespace CG {

inline bool is_one_digit(auto value) {
  return 0 <= value && value < 10;
}

std::ostream& print_grid(
    std::ostream& stream,
    int width,
    int height,
    const std::vector<int>& squares) {
  for (auto y = 0; y < height; ++y) {
    for (auto x = 0; x < width; ++x) {
      const auto value = squares[x + width * y];
      stream << (value == INT::INFTY || value == -1 ? "X " : std::to_string(value))
             << (is_one_digit(value) ? "  " : " ");
    }
    stream << '\n';
  }
  return stream;
}

std::string error_msg_distance_fields(
    int width,
    int height,
    const std::vector<int>& expected,
    const std::vector<int>& actual) {
  std::ostringstream stream;
  stream << "Expected\n";
  print_grid(stream, width, height, expected);

  stream << "\nBut got\n";
  print_grid(stream, width, height, actual);

  return stream.str();
}

} // namespace CG
