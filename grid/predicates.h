#ifndef PREDICATES_H_
#define PREDICATES_H_

#include <cstddef>

namespace CG {

struct TrivialPredicate {
  constexpr bool operator()(std::size_t /* tile_index */, int /* distance] */) const {
    return false;
  }
};

} // namespace CG

#endif // PREDICATES_H_
