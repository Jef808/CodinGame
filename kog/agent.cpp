#include "agent.h"

#include "grid/constants.h"
#include "grid/bfs.h"

#include "kog/actions.h"

#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>

namespace kog {

using Grid = CG::Grid<Tile>;
using Index = Grid::index_type;

Agent::Agent(const Game& game)
    : m_game{game} {
}

namespace  {

/**
 * Compute region consisting of unblocked tiles owned by me, the opponent, or neither.
 * Also compute the boundary of those regions.
 */
void compute_tiles_info(
    const Grid& grid,
    TilesInfo& tiles_info);

/**
 * For each player, record index of tiles occupied by their units. Also compute the number of
 * turns needed for a unit of each player to reach any tile.
 */
void compute_units_info(
    const Grid& grid,
    const TilesInfo& tiles_info,
    UnitsInfo& units_info);

/**
 * Compute the tile regions made up of
 * 1) tiles unreachable by any player's current or future units,
 * 2) for each player, tiles reachable faster by that player's current or future
 *    units than by the other player's,
 * 3) tiles reachable as fast by both player's units.
 */
void compute_territory_info(
    const Grid& grid,
    const UnitsInfo& units_info,
    TerritoryInfo& territory_info);

/**
 * Compute the interior boundary of each player's territory, along
 * with distances from each tiles to those boundaries.
 */
void compute_battlefronts_info(
    const Game::Grid& grid,
    const UnitsInfo& units_info,
    const TerritoryInfo& territory_info,
    BattlefrontsInfo& battlefront_info);

} // namespace

void Agent::compute_turn_info() {
  const Grid& grid = m_game.grid();

  compute_tiles_info(grid, m_tiles_info);
  std::cerr << "Computed tiles info" << std::endl;

  compute_units_info(grid, m_tiles_info, m_units_info);
  std::cerr << "Computed units info" << std::endl;

  compute_territory_info(grid, m_units_info, m_territory_info);
  std::cerr << "Computed territory info" << std::endl;

  compute_battlefronts_info(grid, m_units_info, m_territory_info, m_battlefronts_info);
  std::cerr << "Computed battlefronts info" << std::endl;
}

void Agent::choose_actions(std::ostream& stream) {
  m_actions.clear();

  const auto& grid = m_game.grid();
  static std::set<Index> recyclers;
  recyclers.clear();

  auto number_of_purchases = static_cast<int>(std::floor(m_game.me().matter / 10));

  // Choose defensive recyclers
  for (auto target_index : m_tiles_info.my_boundary) {
    if (number_of_purchases == 0) {
      break;
    }
    const auto& target = grid.at(target_index);
    if (!target.can_build) {
      continue;
    }
    for (auto nbh_index : grid.neighbours_of(target_index)) {
      const auto& nbh_tile = grid.at(nbh_index);
      if (nbh_tile.owner == 0 && nbh_tile.units) {
        make_build_action(stream, target);

        recyclers.insert(target_index);
        --number_of_purchases;
        break;
      }
    }
  }

  std::cerr << "Chose defensive recyclers" << std::endl;

  // Choose spawns
  // std::sort(m_tiles_info.my_boundary.begin(), m_tiles_info.my_boundary.end(), [&](auto a, auto b) {
  //   return m_battlefronts_info.my_frontier_distance_field[a]
  //       < m_battlefronts_info.my_frontier_distance_field[b];
  // });
  // while (number_of_purchases > 0) {
  //   auto ndx = 0;
  //   const auto boundary_size = m_tiles_info.my_boundary.size();
  //   const auto target_index = m_tiles_info.my_boundary[ndx % boundary_size];
  //   const auto& target = grid.at(target_index);
  //   if (!target.can_spawn || recyclers.count(target_index)) {
  //     continue;
  //   }

  //   Spawn{target, grid.at(target_index).units}.print(stream) << ';';

  //   --number_of_purchases;
  // }

  // std::cerr << "Chose spawns" << std::endl;

  // Choose economic recyclers
  // ...
  // ...

  // Choose moves towards the current frontier
  move_towards_frontier(stream);
  std::cerr << "Chose moves" << std::endl;

  make_wait_action(stream) << std::endl;
}

void Agent::move_towards_frontier(std::ostream& stream) {
  const auto& grid = m_game.grid();

  // const auto& my_frontier = m_battlefronts_info.my_frontier;
  for (auto unit : m_units_info.my_units) {
    const auto& from = grid.at(unit);
    const auto& to_index = rand() % (grid.width() * grid.height() - 1);
    const auto& to = grid.at(to_index);
    // const auto& to = grid.at(my_frontier[rand() % my_frontier.size() - 1]);
    const auto number = from.units;
    make_move_action(stream, from, to, number);
  }
}

namespace {

/**
 * Return true if and only if tile at given \p tile_index is unblocked,
 * owned by either me or the opponent and a neighbouring tile
 * which also is unblocked but owned by the opposite player.
 */
inline bool is_on_boundary(const Grid& grid, Index tile_index) {
  const Tile& tile = grid.at(tile_index);
  if (tile.owner == -1 || tile.is_blocked()) {
    const auto& neighbours = grid.neighbours_of(tile_index);
    for (Index nbh_index : neighbours) {
      const Tile& nbh = grid.at(nbh_index);
      if (nbh.owner != tile.owner && !nbh.is_blocked()) {
        return true;
      }
    }
  }
  return false;
}

inline void compute_tiles_info(const Grid& grid,
                               TilesInfo& tiles_info) {
  tiles_info.clear();

  // Mark unblocked tiles as mine, opponent's or neutral.
  std::for_each(grid.begin(), grid.end(), [&](const Tile& tile) {
    if (tile.is_blocked(1)) {
      return;
    }
    const Index index = grid.index_of(tile.x, tile.y);
    if (tile.owner == 1) {
      tiles_info.my_tiles.push_back(index);
    } else if (tile.owner == 0) {
      tiles_info.opp_tiles.push_back(index);
    } else {
      tiles_info.neutral_tiles.push_back(index);
    }
  });

  // Compute boundary of my and opponent's owned region.
  std::copy_if(tiles_info.my_tiles.begin(), tiles_info.my_tiles.end(),
               std::back_inserter(tiles_info.my_boundary), [&](Index index) {
    return (is_on_boundary(grid, index));
  });
  std::copy_if(tiles_info.opp_tiles.begin(), tiles_info.opp_tiles.end(),
               std::back_inserter(tiles_info.opp_boundary), [&](Index index) {
    return (is_on_boundary(grid, index));
  });
}

inline void compute_units_info(const Grid& grid,
                               const TilesInfo& tiles_info,
                               UnitsInfo& units_info) {
  units_info.clear();
  const auto& my_tiles = tiles_info.my_tiles;
  const auto& opp_tiles = tiles_info.opp_tiles;
  auto& my_units = units_info.my_units;
  auto& opp_units = units_info.opp_units;

  // Store index of each unit tile.
  std::copy_if(my_tiles.begin(), my_tiles.end(),
               std::back_inserter(my_units), [&grid](Index index) {
                 return grid.at(index).units > 0;
               });
  std::copy_if(opp_tiles.begin(), opp_tiles.end(),
               std::back_inserter(opp_units), [&grid](Index index) {
                 return grid.at(index).units > 0;
               });

  auto& my_distance_field = units_info.my_distance_field;
  auto& opp_distance_field = units_info.opp_distance_field;

  // Set all tiles to UNVISITED by default.
  std::fill_n(std::back_inserter(my_distance_field), grid.width() * grid.height(),
              CG::INT::UNVISITED);
  std::fill_n(std::back_inserter(opp_distance_field), grid.width() * grid.height(),
              CG::INT::UNVISITED);

  // Preset the distances to 1 for owned but unblocked and unoccupied tiles
  // and to 0 for unit tiles.
  std::for_each(my_tiles.begin(), my_tiles.end(),
                [&d=my_distance_field](Index index) {
                  d[index] = 1;
                });
  std::for_each(opp_tiles.begin(), opp_tiles.end(),
                [&d=opp_distance_field](Index index) {
                  d[index] = 1;
                });
  std::for_each(my_units.begin(), my_units.end(),
                [&d=my_distance_field](Index index) {
                  d[index] = 0;
                });
  std::for_each(opp_units.begin(), opp_units.end(),
                [&d=opp_distance_field](Index index) {
                  d[index] = 0;
                });

  // Compute distances.
  CG::bfs(grid, my_distance_field);
  CG::bfs(grid, opp_distance_field);
}

void compute_territory_info(const Grid& grid, const UnitsInfo& units_info, TerritoryInfo& territory_info) {
  const auto is_unreachable_tile = [](const auto distance_to_tile) {
    return distance_to_tile == CG::INT::UNVISITED || CG::INT::INFTY;
  };

  for (const auto& tile : grid) {
    const auto index = grid.index_of(tile.x, tile.y);
    const auto my_distance = units_info.my_distance_field[index];
    const auto opp_distance = units_info.opp_distance_field[index];

    const auto is_unreachable_by_me = is_unreachable_tile(my_distance);
    const auto is_unreachable_by_opp = is_unreachable_tile(opp_distance);

    // Mark unreachable tiles.
    if (is_unreachable_by_me && is_unreachable_by_opp) {
      territory_info.unreachable_tiles.insert(index);
      continue;
    }
    if (is_unreachable_by_opp) {
      territory_info.my_territory.insert(index);
      continue;
    }
    if (is_unreachable_by_me) {
      territory_info.opp_territory.insert(index);
      continue;
    }

    // Compute distance difference for reachable tiles.
    const auto diff_distance = my_distance - opp_distance;
    if (diff_distance < 0) {
      territory_info.my_territory.insert(index);
    } else if (diff_distance > 0) {
      territory_info.opp_territory.insert(index);
    } else {
      territory_info.neutral_frontier.insert(index);
    }
  }
}

void compute_battlefronts_info(const Game::Grid& grid,
                               const UnitsInfo& units_info,
                               const TerritoryInfo& territory_info,
                               BattlefrontsInfo& battlefronts_info) {
  battlefronts_info.clear();
  const auto& neutral_frontier = territory_info.neutral_frontier;
  const auto& my_territory = territory_info.my_territory;
  const auto& opp_territory = territory_info.opp_territory;

  std::fill_n(
      std::back_inserter(battlefronts_info.my_frontier_distance_field),
      grid.width() * grid.height(),
      CG::INT::UNVISITED);
  std::fill_n(
      std::back_inserter(battlefronts_info.opp_frontier_distance_field),
      grid.width() * grid.height(),
      CG::INT::UNVISITED);

  for (const Index index : my_territory) {
    for (Index nbh : grid.neighbours_of(index)) {
      if (opp_territory.count(nbh) || neutral_frontier.count(nbh)) {
        battlefronts_info.my_frontier.push_back(nbh);
        battlefronts_info.my_frontier_distance_field[index] = 0;
        break;
      }
    }
  }
  for (const Index index : opp_territory) {
    for (Index nbh : grid.neighbours_of(index)) {
      if (my_territory.count(nbh) || neutral_frontier.count(nbh)) {
        battlefronts_info.my_frontier.push_back(nbh);
        battlefronts_info.my_frontier_distance_field[index] = 0;
        break;
      }
    }
  }

  CG::bfs(grid, battlefronts_info.my_frontier_distance_field);
  CG::bfs(grid, battlefronts_info.opp_frontier_distance_field);
}

} // namespace


} // namespace kog
