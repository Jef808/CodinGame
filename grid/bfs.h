#ifndef BFS_H_
#define BFS_H_

#include <iostream>
#include <limits>
#include <queue>
#include <vector>

#include "grid.h"

namespace CG {

struct TrivialPredicate {
  constexpr bool operator()(std::size_t /* tile_index */, int /* distance] */) const {
    return false;
  }
};

// A distance of INFTY will mean the tile is unreachable.
constexpr int INFTY = std::numeric_limits<int>::max();

template <typename Tile, typename TileBlockedPredicate = TrivialPredicate>
  void bfs(
      const Grid<Tile>& grid,
      std::size_t source,
      std::vector<int>& distance_field_out,
      const TileBlockedPredicate& is_blocked = TileBlockedPredicate{}) {
  // Set all distances to `-1`, which will stand for unvisited
  distance_field_out.resize(grid.width() * grid.height());
  std::fill(distance_field_out.begin(), distance_field_out.end(), -1);

  // The queue of tiles to visit, as (tile_index, distance_to_source)
  std::queue<std::size_t> queue;

  if (!is_blocked(source, 0)) {
    queue.push(source);
    distance_field_out[source] = 0;
  }

  while (!queue.empty()) {
    const auto current_index = queue.front();
    queue.pop();
    const auto current_distance = distance_field_out[current_index];

    for (auto neighbour_index : grid.neighbours_of(current_index)) {
      // Already visited nodes have known distance, we skip them.
      if (distance_field_out[neighbour_index] == -1) {
        // If node is unreachable, set its distance to `INFTY`.
        if (is_blocked(neighbour_index, current_distance + 1)) {
          distance_field_out[neighbour_index] = INFTY;
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

template <typename Tile,
          typename IndexIterator,
          typename TileBlockedPredicate = TrivialPredicate>
  void bfs(
      const Grid<Tile>& grid,
      IndexIterator sources_beg,
      IndexIterator sources_end,
      std::vector<int>& distance_field_out,
      const TileBlockedPredicate& is_blocked = TileBlockedPredicate{}) {
  // Set all distances to `-1`, which will stand for unvisited
  distance_field_out.resize(grid.width() * grid.height());
  std::fill(distance_field_out.begin(), distance_field_out.end(), -1);

  // The queue of tiles to visit, as (tile_index, distance_to_source)
  std::queue<std::size_t> queue;

  for (auto it = sources_beg; it != sources_end; ++it) {
    if (!is_blocked(*it, 0)) {
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
        if (is_blocked(neighbour_index, current_distance + 1)) {
          distance_field_out[neighbour_index] = INFTY;
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

} // namespace CG

#endif // BFS_H_
