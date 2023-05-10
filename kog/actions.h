#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "kog/tile.h"

#include "point/point.h"

#include <iosfwd>

namespace kog {

struct Action {
  [[nodiscard]] virtual std::ostream& print(std::ostream&) const;

  virtual ~Action() = default;
};

inline std::ostream& operator<<(std::ostream& os, const Action& action) {
  return action.print(os);
}

struct Move: Action {
  CG::Point source;
  CG::Point target;
  int number;

  Move(const Tile& tile_from, const Tile& tile_to, int number)
      : source{tile_from.x, tile_from.y}
      , target{tile_to.x, tile_to.y}
      , number{number}
  {
  }

  [[nodiscard]] std::ostream& print(std::ostream& os) const override {
    return os << "MOVE "
              << number << ' '
              << source.x << ' ' << source.y << ' '
              << target.x << ' ' << target.y;
  }
};

struct Spawn: Action {
  CG::Point target;
  int number;

  Spawn(const Tile& tile, int number)
      : target{tile.x, tile.y}
      , number{number}
  {
  }

  [[nodiscard]] std::ostream& print(std::ostream& os) const override {
    return os << "SPAWN "
              << number << ' ' << target.x << ' ' << target.y;
  }
};

struct Build: Action {
  CG::Point target;

  Build(const Tile& tile)
      : target{tile.x, tile.y}
  {
  }

  [[nodiscard]] std::ostream& print(std::ostream& os) const override {
    return os << "BUILD " << target.x << ' ' << target.y;
  }
};

} // namespace kog

#endif // ACTIONS_H_
