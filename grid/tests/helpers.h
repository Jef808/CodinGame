#ifndef HELPERS_H_
#define HELPERS_H_

#include <algorithm>
#include <sstream>
#include <vector>

namespace CG {

std::string error_msg_distance_fields(int width,
                                      int height,
                                      const std::vector<int>& actual,
                                      const std::vector<int>& expected);

inline std::ostream& output_tile(std::ostream& stream, const std::size_t index, std::size_t width) {
  return stream << '('
                << index % width << ", " << index / width << "), ";
}


} // namespace CG

#endif // HELPERS_H_
