#ifndef AGENT_H_
#define AGENT_H_

#include <string>
#include <vector>

#include "kog/game.h"
#include "kog/territory_info.h"
#include "kog/tiles_info.h"
#include "kog/units_info.h"
#include "kog/battlefronts_info.h"

namespace kog {

class Agent;

namespace debug {
enum class Flag;
void serialize(std::ostream& stream, const Agent& agent, Flag flag);
} // namespace debug::agent


class Agent {
 public:
  using Grid = Game::Grid;
  using Index = Game::Grid::index_type;

  explicit Agent(const Game& game);

  void compute_turn_info();

  void choose_actions(std::ostream& stream);

 private:
  const Game& m_game;

  TilesInfo m_tiles_info;
  UnitsInfo m_units_info;
  TerritoryInfo m_territory_info;
  BattlefrontsInfo m_battlefronts_info;

  std::vector<std::string> m_actions;

  void choose_move_actions(std::ostream& stream) const;
  void move_towards_frontier(std::ostream& stream) const;

  void choose_spawn_actions(std::ostream& stream) const;
  void spawn_towards_frontier(std::ostream& stream) const;

  void choose_recycler_actions(std::ostream& stream) const;

  // debug
  friend void debug::serialize(std::ostream&, const Agent&, debug::Flag);
};

} // namespace kog

#endif // AGENT_H_
