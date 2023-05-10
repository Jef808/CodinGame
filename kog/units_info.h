#ifndef UNITS_INFO_H_
#define UNITS_INFO_H_

#include <vector>

#include "kog/game.h"


namespace kog {

struct UnitsInfo {
  std::vector<Game::Grid::index_type> my_units;
  std::vector<Game::Grid::index_type> opp_units;
  std::vector<int> my_distance_field;
  std::vector<int> opp_distance_field;

  void clear() {
    my_units.clear();
    opp_units.clear();
    my_distance_field.clear();
    opp_distance_field.clear();
  }
};

} // namespace kog

#endif // UNITS_INFO_H_
