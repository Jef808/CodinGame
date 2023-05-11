#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "kog/tile.h"

#include "point/point.h"

#include <iosfwd>

namespace kog {

struct Action {
    Action() = default;
    [[nodiscard]] Action(const Action&) = default;
    Action(Action&&) = delete;
    Action& operator=(const Action&) = default;
    Action& operator=(Action&&) = delete;

    virtual std::ostream& print(std::ostream& stream) const { return stream; };

    virtual ~Action() = default;
};

inline std::ostream& operator<<(std::ostream& stream, const Action& action) {
  return action.print(stream);
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

  [[nodiscard]] std::ostream& print(std::ostream& stream) const override {
    return stream << "MOVE "
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

  [[nodiscard]] std::ostream& print(std::ostream& stream) const override {
    return stream << "SPAWN "
              << number << ' ' << target.x << ' ' << target.y;
  }
};

struct Build: Action {
  CG::Point target;

  Build(const Tile& tile)
      : target{tile.x, tile.y}
  {
  }

  [[nodiscard]] std::ostream& print(std::ostream& stream) const override {
    return stream << "BUILD " << target.x << ' ' << target.y;
  }
};

struct Wait: Action {
  Wait() = default;

  [[nodiscard]] std::ostream& print(std::ostream& stream) const override {
    return stream << "WAIT";
  }
};

inline std::ostream& make_move_action(std::ostream& stream, const Tile& tile_from, const Tile& tile_to, int number) {
  return Move{tile_from, tile_to, number}.print(stream) << ';';
}

inline std::ostream& make_spawn_action(std::ostream& stream, const Tile& tile, int number) {
  return Spawn{tile, number}.print(stream) << ';';
}

inline std::ostream& make_build_action(std::ostream& stream, const Tile& tile) {
  return Build{tile}.print(stream) << ';';
}

inline std::ostream& make_wait_action(std::ostream& stream) {
  return Wait{}.print(stream) << ';';
}

} // namespace kog

#endif // ACTIONS_H_
