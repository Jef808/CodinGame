#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <limits>

namespace CG {

// A distance of INFTY will mean the tile is unreachable.
struct INT {
  static constexpr int INFTY = std::numeric_limits<int>::max();
  static constexpr int UNVISITED = -1;
};

template <typename Grid>
struct INDEX {
  static constexpr typename Grid::index_type NONE = std::numeric_limits<typename Grid::index_type>::max();
};

} // namespace CG

#endif // CONSTANTS_H_
