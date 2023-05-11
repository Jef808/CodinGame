#ifndef GAME_H_
#define GAME_H_

#include <iosfwd>

#include "player.h"
#include "tile.h"
#include "grid/grid.h"

namespace kog {

class Game {
 public:
  using Grid = CG::Grid<Tile>;

  Game() = default;
  Game(Grid&& grid);

  void initial_input(std::istream& stream);
  void turn_input(std::istream& stream);

  void output_grid(std::ostream& stream);

  [[nodiscard]] const Grid& grid() const { return m_grid; }
  [[nodiscard]] const Player& me() const { return m_me; }
  [[nodiscard]] const Player& opp() const { return m_opp; }

 private:
  Grid m_grid;
  Player m_me;
  Player m_opp;
};

} // namespace kog

#endif // GAME_H_
