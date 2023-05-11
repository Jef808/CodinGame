#ifndef BATTLEFRONTS_INFO_H_
#define BATTLEFRONTS_INFO_H_

#include <vector>

#include "game.h"

namespace kog {

struct BattlefrontsInfo {
  std::vector<Game::Grid::index_type> my_frontier;
  std::vector<Game::Grid::index_type> opp_frontier;
  std::vector<int> my_frontier_distance_field;
  std::vector<int> opp_frontier_distance_field;

  void clear() {
    my_frontier.clear();
    opp_frontier.clear();
    my_frontier_distance_field.clear();
    opp_frontier_distance_field.clear();
  }
};

} // namespace kog

#endif // BATTLEFRONTS_INFO_H_
