#ifndef AGENT_DEBUG_FLAGS_H_
#define AGENT_DEBUG_FLAGS_H_

#include "kog/agent.h"
#include "grid/constants.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

namespace kog::debug {

enum class Flag {
  Tiles_Info,
  Units_Info,
  Territory_Info,
  Battlefronts_Info
};

/**
 * Serialize the data of the \p agent specified by the \p flag.
 */
void serialize(std::ostream& stream, const Agent& agent, Flag flag);

inline std::ostream& operator<<(std::ostream& stream, const CG::Point& point) {
  return stream << point.x << ',' << point.y;
}

inline std::ostream& serialize(std::ostream& stream,
                               const Agent::Grid& grid,
                               const std::vector<Agent::Index>& indices) {
  std::transform(indices.begin(), indices.end(), std::ostream_iterator<CG::Point>{stream, " "},
                 [&grid](const Agent::Index index) {
                   return grid.at(index);
                 });
  return stream;
}

inline std::ostream& serialize(std::ostream& stream,
                               const Agent::Grid& grid,
                               const std::vector<int>& numbers) {
  std::transform(numbers.begin(), numbers.end(), std::ostream_iterator<std::string>{stream, " "},
                 [](auto number) {
                   return number == CG::INT::INFTY ?
                       "X" : number == CG::INT::UNVISITED ?
                       "U" :
                       std::to_string(number);
                 });
  return stream;
}

template <typename IndexInputIterator>
inline std::ostream& serialize(std::ostream& stream,
                               const Agent::Grid& grid,
                               IndexInputIterator beg,
                               IndexInputIterator end) {
  std::transform(beg, end, std::ostream_iterator<CG::Point>{stream, " "},
                 [&grid](const Agent::Index index) {
                   return grid.at(index);
                 });
  return stream;
}

template <typename IndexInputIterator>
inline std::ostream& serialize_unordered(std::ostream& stream,
                                         const Agent::Grid& grid,
                                         IndexInputIterator beg,
                                         IndexInputIterator end) {
  std::vector<typename IndexInputIterator::value_type> indices{beg, end};
  std::sort(indices.begin(), indices.end());
  return serialize(stream, grid, beg, end);
}

template <typename Tile>
inline std::ostream& serialize(std::ostream& stream,
                               const CG::Grid<Tile>& grid,
                               const TilesInfo& tiles_info) {
  return stream
      << "my_tiles:\n";        serialize(stream, grid,
                                         tiles_info.my_tiles.begin(),
                                         tiles_info.my_tiles.end())
      << "\nopp_tiles:\n";     serialize(stream, grid,
                                         tiles_info.opp_tiles.begin(),
                                         tiles_info.opp_tiles.end())
      << "\nneutral_tiles:\n"; serialize(stream, grid,
                                         tiles_info.neutral_tiles.begin(),
                                         tiles_info.neutral_tiles.end())
      << "\nmy_boundary:\n";   serialize(stream, grid,
                                         tiles_info.my_boundary.begin(),
                                         tiles_info.my_boundary.end())
      << "\nopp_boundary:\n";  serialize(stream, grid,
                                         tiles_info.opp_boundary.begin(),
                                         tiles_info.opp_boundary.end());
}

template <typename Tile>
inline std::ostream& serialize(std::ostream& stream,
                               const CG::Grid<Tile>& grid,
                               const UnitsInfo& units_info) {
  return stream
      << "my_units:\n";             serialize(stream, grid,
                                              units_info.my_units.begin(),
                                              units_info.my_units.end())
      << "\nopp_units:\n";          serialize(stream, grid,
                                              units_info.opp_units.begin(),
                                              units_info.opp_units.end())
      << "\nmy_distance_field:\n";  serialize(stream, grid,
                                              units_info.my_distance_field.begin(),
                                              units_info.my_distance_field.end())
      << "\nopp_distance_field:\n"; serialize(stream, grid, units_info.opp_distance_field.begin(),
                                              units_info.opp_distance_field.end());
}

template <typename Tile>
inline std::ostream& serialize(std::ostream& stream,
                               const CG::Grid<Tile>& grid,
                               const TerritoryInfo& territory_info) {
  return stream
      << "unreachable_tiles:\n";  serialize_unordered(stream, grid,
                                                      territory_info.unreachable_tiles.begin(),
                                                      territory_info.unreachable_tiles.end())
      << "\nmy_territory:\n";     serialize_unordered(stream, grid,
                                                      territory_info.my_territory.begin(),
                                                      territory_info.my_territory.end())
      << "\nopp_territory:\n";    serialize_unordered(stream, grid,
                                                      territory_info.opp_territory.begin(),
                                                      territory_info.opp_territory.end())
      << "\nneutral_frontier:\n"; serialize_unordered(stream, grid,
                                                      territory_info.neutral_frontier.begin(),
                                                      territory_info.neutral_frontier.end());
}

template <typename Tile>
inline std::ostream& serialize(std::ostream& stream,
                               const CG::Grid<Tile>& grid,
                               const BattlefrontsInfo& battlefronts_info) {
  return stream
      << "my_frontier:\n";                   serialize(stream, grid,
                                                       battlefronts_info.my_frontier.begin(),
                                                       battlefronts_info.my_frontier.end())
      << "\nopp_frontier:\n";                serialize(stream, grid,
                                                       battlefronts_info.opp_frontier.begin(),
                                                       battlefronts_info.opp_frontier.end())
      << "\nmy_frontier_distance_field:\n";  serialize(stream, grid,
                                                       battlefronts_info.my_frontier_distance_field.begin(),
                                                       battlefronts_info.my_frontier_distance_field.end())
      << "\nopp_frontier_distance_field:\n"; serialize(stream, grid,
                                                       battlefronts_info.opp_frontier_distance_field.begin(),
                                                       battlefronts_info.opp_frontier_distance_field.end());
}

inline void serialize(std::ostream& stream, const Agent& agent, Flag flag) {
  const auto& grid = agent.m_game.grid();
  switch (flag) {
    case Flag::Tiles_Info: {
      serialize(stream, grid, agent.m_tiles_info);
      break;
    }
    case Flag::Units_Info: {
        serialize(stream, grid, agent.m_units_info);
        break;
    }
    case Flag::Territory_Info: {
        serialize(stream, grid, agent.m_territory_info);
        break;
    }
    case Flag::Battlefronts_Info: {
        serialize(stream, grid, agent.m_battlefronts_info);
        break;
    }
    default:
      throw "unimplemented flag";
  }
}

} // namespace kog

#endif // AGENT_DEBUG_FLAGS_H_
