#ifndef AGENT_H_
#define AGENT_H_

#include <vector>

#include "kog/actions.h"
#include "kog/game.h"
#include "kog/territory_info.h"
#include "kog/tiles_info.h"
#include "kog/boundary_info.h"
#include "kog/units_info.h"
#include "kog/battlefront_info.h"

namespace kog {

inline void output_tile(std::ostream& stream, const std::size_t index, std::size_t width) {
  stream << '('
         << index % width << ", " << index / width << "), ";
}

class Agent {
 public:
  using Grid = Game::Grid;
  using Index = Game::Grid::index_type;

  explicit Agent(const Game& game);

  void reset_turn_info();

  void init_turn_info();

  void compute_turn_info();

  void choose_actions();

  void output_actions(std::ostream& stream) const;

  void debug(std::ostream& stream) const {
    stream << "My number of projected tiles: " << m_territory_info.my_projected_tiles.size() << '\n';
    stream << "Opp number of projected tiles: " << m_territory_info.opp_projected_tiles.size() << '\n';
    stream << "My frontier:\n";
    for (const auto& tile : m_battlefronts_info.my_frontier) {
      output_tile(stream, tile, m_game.grid().width());
    }
    stream << std::endl;
  }

 private:
  const Game& m_game;

  TilesInfo m_tiles_info;
  BoundaryInfo m_boundary_info;
  UnitsInfo m_units_info;
  TerritoryInfo m_territory_info;
  BattlefrontsInfo m_battlefronts_info;

  std::vector<Action> m_actions;

  void choose_move_actions();

  void choose_spawn_actions();

  void choose_recycler_actions();
  void move_towards_frontier();
};

} // namespace kog

#endif // AGENT_H_
