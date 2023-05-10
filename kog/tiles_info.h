#ifndef TILES_INFO_H_
#define TILES_INFO_H_

#include <vector>

#include "kog/game.h"

namespace kog {

struct TilesInfo {
  std::vector<Game::Grid::index_type> my_tiles;
  std::vector<Game::Grid::index_type> opp_tiles;
  std::vector<Game::Grid::index_type> neutral_tiles;
  std::vector<Game::Grid::index_type> my_boundary;
  std::vector<Game::Grid::index_type> opp_boundary;

  void clear() {
    my_tiles.clear();
    opp_tiles.clear();
    neutral_tiles.clear();
    my_boundary.clear();
    opp_boundary.clear();
  }
};

} // namespace kog

#endif // TILES_INFO_H_
