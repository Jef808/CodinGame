#ifndef BOUNDARY_INFO_H_
#define BOUNDARY_INFO_H_

#include "kog/game.h"

namespace kog {

struct BoundaryInfo {
  std::vector<Game::Grid::index_type> my_boundary;
  std::vector<Game::Grid::index_type> opp_boundary;

  void clear() {
    my_boundary.clear();
    opp_boundary.clear();
  }
};

} // namespace kog

#endif // BOUNDARY_INFO_H_
