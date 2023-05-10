#ifndef BFS_H_
#define BFS_H_

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <vector>

#include "grid.h"
#include "constants.h"

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

template <typename Tile>
void bfs(const Grid<Tile>& grid,
         std::vector<int>& distance_field) {
  using Grid = Grid<Tile>;
  using Index = typename Grid::index_type;

  std::queue<Index> queue;

  std::for_each(distance_field.begin(), distance_field.end(),
                [&queue, n=0](auto prior_distance) mutable {
                  if (prior_distance != CG::INT::UNVISITED && prior_distance != CG::INT::INFTY) {
                    queue.push(n++);
                  }
                });

  while (!queue.empty()) {
    const auto& current_index = queue.front();
    queue.pop();
    const auto current_distance = distance_field[current_index];

    for (auto neighbour_index: grid.neighbours_of(current_index)) {
      const auto nbh_distance = current_distance + 1;

      // Skip already visited nodes.
      if (distance_field[neighbour_index] != INT::UNVISITED) {
        continue;
      }

      // Set distance to infinity for neighbours blocked at that distance.
      if (grid.at(neighbour_index).is_blocked(nbh_distance)) {
        distance_field[neighbour_index] = INT::INFTY;
        continue;
      }

      // Otherwise adjust distance field and add neighbour to queue.
      distance_field[neighbour_index] = nbh_distance;
      queue.push(neighbour_index);
    }
  }
}

} // namespace CG

#endif // BFS_H_
