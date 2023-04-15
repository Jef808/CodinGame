#ifndef INITIALISE_NEIGHBOURS_H_
#define INITIALISE_NEIGHBOURS_H_

#include <vector>

namespace CG::impl {

template <typename Index>
void compute_tiles_neighbours(
    Index width,
    Index height,
    std::vector<std::vector<Index>>& neighbours_by_index) {
  for (auto y = 0; y < height; ++y) {
    for (auto x = 0; x < width; ++x) {
      auto& nbhs = neighbours_by_index.emplace_back();
      auto tile_index = x + width * y;
      if (x > 0) {
        nbhs.push_back(tile_index - 1);
      }
      if (y > 0) {
        nbhs.push_back(tile_index - width);
      }
      if (x < width - 1) {
        nbhs.push_back(tile_index + 1);
      }
      if (y < height - 1) {
        nbhs.push_back(tile_index + width);
      }
    }
  }
}

}

#endif // INITIALISE_NEIGHBOURS_H_
