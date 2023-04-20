#ifndef BFS_H_
#define BFS_H_

#include <array>
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

#include "grid.h"
#include "constants.h"
#include "predicates.h"

namespace CG {

template <typename Tile,
          typename IndexIterator>
void bfs(
    const Grid<Tile>& grid,
    IndexIterator sources_beg,
    IndexIterator sources_end,
    std::vector<int>& distance_field_out) {
  // Set all distances to `-1`, which will stand for unvisited
  distance_field_out.resize(grid.width() * grid.height());
  std::fill(distance_field_out.begin(),
            distance_field_out.end(),
            INT::UNVISITED);

  // The queue of tiles to visit, as (tile_index, distance_to_source)
  std::queue<std::size_t> queue;

  for (auto it = sources_beg; it != sources_end; ++it) {
    if (!grid.at(*it).is_blocked()) {
      queue.push(*it);
      distance_field_out[*it] = 0;
    }
  }

  while (!queue.empty()) {
    const auto& current_index = queue.front();
    queue.pop();
    const auto current_distance = distance_field_out[current_index];

    for (auto neighbour_index : grid.neighbours_of(current_index)) {
      // Already visited nodes have known distance, we skip them.
      if (distance_field_out[neighbour_index] == -1) {
        // If node is unreachable, set its distance to `INFTY`.
        if (grid.at(neighbour_index).is_blocked(current_distance + 1)) {
          distance_field_out[neighbour_index] = INT::INFTY;
        }
        else {
          // Otherwise increment distance by 1 and add neighbour to queue.
          distance_field_out[neighbour_index] = current_distance + 1;
          queue.push(neighbour_index);
        }
      }
    }
  }
}

template <typename Tile>
inline void bfs(
    const Grid<Tile>& grid,
    typename Grid<Tile>::index_type source,
    std::vector<int>& distance_field_out) {
  std::array<typename Grid<Tile>::index_type, 1> _source = {source};
  bfs(grid, _source.begin(), _source.end(), distance_field_out);
}

} // namespace CG

#endif // BFS_H_
