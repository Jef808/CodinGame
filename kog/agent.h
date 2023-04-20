#ifndef AGENT_H_
#define AGENT_H_

#include <deque>
#include <set>
#include <vector>

#include "game.h"
#include "grid/voronoi.h"

namespace kog {

class Agent {
 public:
  using Index = Game::Grid::index_type;

  explicit Agent(const Game& game);

  void clear_info();

  void compute_turn_info();

  void debug(std::ostream& stream) const {
    stream << "Battlefront:\n";
    for (auto ndx : m_battlefront) {
      stream << ndx << ' ';
    }
    stream << std::endl;
  }

 private:
  const Game& m_game;

  int m_my_units_total{0};
  int m_opp_units_total{0};
  std::vector<Index> m_my_tiles;
  std::vector<Index> m_opp_tiles;
  std::vector<Index> m_my_units;
  std::vector<Index> m_opp_units;
  // NOTE: Could write a custom iterator chaining both my_units and opp_units instead
  std::vector<Index> m_units;
  std::set<Index> m_my_boundary;
  std::set<Index> m_opp_boundary;
  std::vector<CG::VoronoiTileDescriptor<Tile>> m_voronoi;
  std::set<Index> m_battlefront;
};

} // namespace kog

#endif // AGENT_H_
