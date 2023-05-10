#ifndef TILE_H_
#define TILE_H_

#include "point/point.h"

#include <iostream>

namespace kog {

struct Tile {
  int x{ -1 };
  int y{ -1 };
  int scrap_amount{ 0 };
  int owner{ -1 };
  int units{ 0 };
  bool recycler{ false };
  bool can_build{ false };
  bool can_spawn{ false };
  bool in_range_of_recycler{ false };

  /**
   * Check if tile can be used as part of a path.
   *
   * \param tile The tile in question.
   * \param distance The number of turns until the tile would be used as part of the path.
   */
  [[nodiscard]] bool is_blocked(int distance = 1) const {
    return recycler || scrap_amount <= (in_range_of_recycler * distance);
  }

  operator CG::Point() { return {x, y}; }
};

inline std::istream& operator>>(std::istream& stream, Tile& tile) {
  return stream >> tile.scrap_amount
                >> tile.owner
                >> tile.units
                >> tile.recycler
                >> tile.can_build
                >> tile.can_spawn
                >> tile.in_range_of_recycler;
}

inline std::ostream& operator<<(std::ostream& stream, const Tile& tile) {
  return stream << "(x, y) = (" << tile.x << ", " << tile.y << ")\n"
                << "scrap_amount = " << tile.scrap_amount << '\n'
                << "owner = " << tile.owner << '\n'
                << "units = " << tile.units << '\n'
                << "recycler = " << tile.recycler;
}

} // namespace kog

#endif // TILE_H_
