#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <limits>

namespace CG {

// A distance of INFTY will mean the tile is unreachable.
// TODO Template over DistanceT
struct INT {
  static constexpr int INFTY = std::numeric_limits<int>::max();
  static constexpr int UNVISITED = -1;
};

template <typename IndexT>
struct INDEX {
  static constexpr IndexT NONE = std::numeric_limits<IndexT>::max();
};

} // namespace CG

#endif // CONSTANTS_H_
