#ifndef GRID_H_
#define GRID_H_

#include <vector>

namespace CG {

namespace impl {

template <typename Index>
void compute_tiles_neighbours(
    Index width,
    Index height,
    std::vector<std::vector<Index>>& neighbours_by_index);

} // namespace impl

template <typename Tile>
class Grid {
 public:
  using tile_type = Tile;
  using index_type = typename std::vector<tile_type>::size_type;

  Grid() = default;

   Grid(index_type width, index_type height)
      : m_width{width}, m_height{height} {
    impl::compute_tiles_neighbours(width, height, m_neighbours_by_index);
  }

  void set_dimensions(index_type width, index_type height) {
    m_width = width;
    m_height = height;
    impl::compute_tiles_neighbours(width, height, m_neighbours_by_index);
  }

  void set_tiles(std::vector<Tile>&& tiles) { m_tiles = tiles; }
  void swap_tiles(std::vector<Tile>& tiles) { std::swap(m_tiles, tiles); }

  [[nodiscard]] index_type width() const { return m_width; }

  [[nodiscard]] index_type height() const { return m_height; }

  [[nodiscard]] index_type index_of(int x, int y) const { return x + m_width * y; }

  [[nodiscard]] const std::vector<index_type>& neighbours_of(index_type tile_index) const {
    return m_neighbours_by_index[tile_index];
  }

  const Tile& at(index_type index) const { return m_tiles[index]; }

  typename std::vector<Tile>::const_iterator begin() const { return m_tiles.begin(); }

  typename std::vector<Tile>::const_iterator end() const { return m_tiles.end(); }

 private:
  index_type m_width{0};
  index_type m_height{0};
  std::vector<Tile> m_tiles;
  std::vector<std::vector<index_type>> m_neighbours_by_index;
};


} // namespace CG

#include "impl/compute_neighbours.h"

#endif // GRID_H_
