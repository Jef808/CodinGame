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

} // namespace CG

#endif // HELPERS_H_
