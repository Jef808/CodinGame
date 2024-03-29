#ifndef TERRITORY_INFO_H_
#define TERRITORY_INFO_H_

#include <unordered_set>

#include "game.h"

namespace kog {

struct TerritoryInfo {
  std::unordered_set<Game::Grid::index_type> unreachable_tiles;
  std::unordered_set<Game::Grid::index_type> my_territory;
  std::unordered_set<Game::Grid::index_type> opp_territory;
  std::unordered_set<Game::Grid::index_type> neutral_frontier;

  void clear() {
    my_territory.clear();
    opp_territory.clear();
    unreachable_tiles.clear();
    neutral_frontier.clear();
  }
};

} // namespace kog

#endif // TERRITORY_INFO_H_
