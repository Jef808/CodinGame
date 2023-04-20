#ifndef VORONOI_H_
#define VORONOI_H_

#include "grid.h"
#include "constants.h"
#include "predicates.h"

#include <algorithm>
#include <limits>
#include <queue>

namespace CG {

template <typename Tile>
struct VoronoiTileDescriptor {
  using Index = typename Grid<Tile>::index_type;

  int distance{INT::INFTY};
  std::vector<Index> sites;
};

template <typename Tile,
          typename IndexIterator>
void generate_voronoi_diagram(
    const Grid<Tile>& grid,
    IndexIterator sites_beg,
    IndexIterator sites_end,
    std::vector<VoronoiTileDescriptor<Tile>>& voronoi_out) {
  using Grid = Grid<Tile>;
  using Index = typename Grid::index_type;

  voronoi_out.resize(grid.width() * grid.height());
  std::fill(voronoi_out.begin(),
            voronoi_out.end(),
            VoronoiTileDescriptor<Tile>{INT::INFTY, {}});

  std::queue<Index> queue;

  for (auto it = sites_beg; it != sites_end; ++it) {
    if (!grid.at(*it).is_blocked()) {
      queue.push(*it);
      voronoi_out[*it].distance = 0;
      voronoi_out[*it].sites.emplace_back(*it);
    }

    while (!queue.empty()) {
      const auto& current_index = queue.front();
      queue.pop();
      const auto current_distance = voronoi_out[current_index].distance;

      for (const auto& neighbour_index: grid.neighbours_of(current_index)) {
        auto tentative_distance = current_distance + 1;
        if (grid.at(neighbour_index).is_blocked(tentative_distance)) {
          continue;
        }
        if (tentative_distance <= voronoi_out[neighbour_index].distance) {
          voronoi_out[neighbour_index].sites.push_back(neighbour_index);
        }
        if (tentative_distance < voronoi_out[neighbour_index].distance) {
          voronoi_out[neighbour_index].distance = tentative_distance;
          queue.push(neighbour_index);
        }
      }
    }
  }
}


} // namespace CG

#endif // VORONOI_H_
