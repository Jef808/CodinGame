#include "gridutils.h"

#include <iostream>

namespace TowerDefense {

constexpr double MAX_HEAT_RADIUS = 6.0;

namespace {
std::vector<int> nb_shortest_paths(const Game& game) {
  std::vector<int> ret;

  for (auto y = 0; y < game.height(); ++y) {
    for (auto x = 0; x < game.width(); ++x) {
      int n = game.at(x, y) & Grid::Tile::Plateau ? 0 : 1;
      ret.push_back(n);
    }
  }

   return ret;
}
} // namespace

std::vector<double> heatmap(const Game& game) {
  std::vector<int> _nb_shortest_paths = nb_shortest_paths(game);
  std::vector<double> heatmap;

  for (auto y = 0; y < game.height(); ++y) {
    for (auto x = 0; x < game.width(); ++x) {
      const double heat = -1.0 + (game.at(x, y) & Grid::Tile::Plateau);
      heatmap.push_back(heat);
    }
  }

  for (auto py = 0; py < game.height(); ++py) {
    for (auto px = 0; px < game.width(); ++px) {
      const Point p{ px, py };

      // Only spread heat from Canyon tiles
      if (game.at(p) & Grid::Tile::Plateau) {
        continue;
      }

      // Less heat if source is less busy
      const double weight = 1.0 / _nb_shortest_paths[px + game.width() * py];

      const auto qx_beg = std::max(0, px - (int)MAX_HEAT_RADIUS);
      const auto qx_end = std::min(game.width(), px + (int)MAX_HEAT_RADIUS);
      const auto qy_beg = std::max(0, py - (int)MAX_HEAT_RADIUS);
      const auto qy_end = std::min(game.height(), py + (int)MAX_HEAT_RADIUS);

      for (auto qy = qy_beg; qy < qy_end; ++qy) {
        for (auto qx = qx_beg; qx < qx_end; ++qx) {
          const Point q{ qx, qy };
          const double _distance = distance(p, q);

          // Only accumulate heat at points distinct of p within the ball of radius MAX_HEAT_RADIUS
          if (_distance < 1 || _distance > MAX_HEAT_RADIUS)
            continue;

          // Only accumulate heat to Plateau tiles
          if (game.at(qx, qy) & Grid::Tile::Canyon)
            continue;

          heatmap[qx + game.width() * qy] += weight / _distance;
        }
      }
    }
  }

  return heatmap;
}


} // namespace TowerDefense
