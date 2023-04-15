#include "agent.h"

namespace kog {

namespace {

using Grid = CG::Grid<Tile>;
using Index = Grid::index_type;

/**
 * Check if tile can be used as part of a path.
 *
 * \param grid The current grid.
 * \param tile The index of the tile in question.
 * \param distance The distance of the tile from the start of the path.
 */
inline bool is_traversable(const Grid& grid, Index tile_index, double distance = 1.0) {
  return is_traversable(grid.at(tile_index), distance);
}

/**
 * Check if tile is on the boundary of my or opponent's owned tiles.
 */
bool is_on_boundary(const Grid& grid, const Tile& tile) {
  if (tile.owner != -1 && is_traversable(tile)) {
    const auto index = grid.index_of(tile.x, tile.y);
    const auto& neighbours = grid.neighbours_of(index);
    for (auto nbh : neighbours) {
      const auto& nbh_tile = grid.at(nbh);
      if (nbh_tile.owner != tile.owner && is_traversable(nbh_tile)) {
        return true;
      }
    }
  }
  return false;
}

} // namespace

Agent::Agent(const Game& game)
    : m_game{game}
    , m_units_voronoi{game.grid()}
{
}

void Agent::clear_info() {
  m_my_units_total = 0;
  m_opp_units_total = 0;
  m_my_tiles.clear();
  m_opp_tiles.clear();
  m_my_units.clear();
  m_opp_units.clear();
  m_my_boundary.clear();
  m_opp_boundary.clear();
}

void Agent::compute_turn_info() {
  const Grid& grid = m_game.grid();

  for (const auto& tile : grid) {
    auto index = grid.index_of(tile.x, tile.y);

    // Store my tiles
    if (tile.owner == 1) {
      m_my_tiles.push_back(index);
      if (tile.units > 0) {
        m_my_units_total += tile.units;
        m_units.emplace_back(m_my_units.emplace_back(index));
      }
    }

    // Store opponent tiles
    if (tile.owner == 0) {
      m_opp_tiles.push_back(index);
      if (tile.units > 0) {
        m_my_units_total += tile.units;
        m_units.emplace_back(m_opp_units.emplace_back(index));
      }
    }

    // Compute boundaries
    if (is_on_boundary(grid, tile)) {
      (tile.owner == 1 ? m_my_boundary : m_opp_boundary).insert(index);
    }
  }

  // Compute Voronoi diagrams where the sites are the tiles with units.
  m_units_voronoi.reset(m_units.begin(), m_units.end());

  // Compute the current battlefront.
  //
  // FIXME
  // Since we only visit each tile at most once in the bfs, they can have
  // at most one parent only.
  // Instead, use same bfs algorithm but run voronoi twice: once for
  // my units and once for opponent's units. Can then take difference of
  // the distance fields and the frontier is going to be the zero locus
  // union with the {+1, -1} locus not touching the 0 locus.
  for (const auto& tile : grid) {
    const auto tile_index = grid.index_of(tile.x, tile.y);
    const auto& sites = m_units_voronoi.get_closest_sites(tile_index);

    // Find the tiles at the boundary of sites owned by both players.
    if (sites.size() < 2) {
      continue;
    }

    bool is_on_my_territory = false;
    bool is_on_opp_territory = false;

    for (Index site_index : sites) {
      const Tile& site = m_game.grid().at(site_index);
      if (site.owner == 1) {
        is_on_my_territory = true;
      }
      else {
        is_on_opp_territory = true;
      }

      if (is_on_my_territory & is_on_opp_territory) {
        m_battlefront.insert(tile_index);
      }
    }
  }
  // The minimal-weight match solver expects the data of all distances
  // between unit tiles of opposite players.
  // So w(i, j) = distance between my i'th unit tile and
  // opponent's j'th unit tile.
  // m_matching_solver.set_sizes(/* size_A = */m_my_units.size(),
  //                             /* size_B = */m_opp_units.size());
  // double* data = m_matching_solver.data();
  // for (auto tile : m_my_units) {
  //   for (auto opp_tile : m_opp_units) {
  //     *data++ = m_my_distances[tile][opp_tile];
  //   }
  // }

  // Assign opponent units to our units, from closest pairs to farthest.
  // auto units_assigned = 0;
  // while (units_assigned < m_my_units_total) {
  //   // Find the closest pair of opposing units.

  // }
}

} // namespace kog
