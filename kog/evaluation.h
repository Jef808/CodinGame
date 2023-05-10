#ifndef EVALUATION_H_
#define EVALUATION_H_

#include "kog/game.h"
#include "kog/voronoi_helper.h"

namespace kog {

double evaluate(const BattlefrontInfo& bfi) {
  const double my_number_tiles = bfi.my_projected_tiles;
  const double opp_number_tiles = bfi.opp_projected_tiles;
  const double tile_count_score = (my_number_tiles - opp_number_tiles)
                                  / (my_number_tiles + opp_number_tiles);
  return tile_count_score;
}

} // namespace kog

#endif // EVALUATION_H_
