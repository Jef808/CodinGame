#ifndef DIRECTION_FIELD_H_
#define DIRECTION_FIELD_H_

#include <algorithm>
#include <vector>

namespace kog {

namespace {

enum class Direction {
  WEST,
  NORTH,
  EAST,
  SOUTH
};

} // namespace

template <typename Index>
using DirectionField = std::vector<Direction>;

template <typename GridT>
typename GridT::index_type move_towards(const GridT& grid,
                                        typename GridT::index_type index,
                                        Direction direction) {
  const auto width = grid.width();
  switch (direction) {
    case Direction::WEST: return index - 1;
    case Direction::NORTH: return index - width;
    case Direction::EAST: return index + 1;
    case Direction::SOUTH: return index + width;
  }
}

template <typename GridT, typename UnitsInputIterator, typename DirectionsInputIterator>
const std::vector<typename GridT::index_type>&
move_units(const GridT& grid,
           UnitsInputIterator units_begin,
           UnitsInputIterator units_end,
           DirectionsInputIterator directions_begin) {
  static std::vector<typename GridT::index_type> new_tiles;
  new_tiles.clear();

  auto output = std::back_inserter(new_tiles);

  std::transform(units_begin, units_end, directions_begin, output,
                 [&grid](auto unit, auto direction) {
                   return move_towards(grid, unit, direction);
                 });

  return new_tiles;
}

} // namespace kog

#endif // DIRECTION_FIELD_H_
