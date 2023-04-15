#include "game.h"

#include <iostream>

namespace kog {

void Game::initial_input(std::istream& stream) {
  int width;
  int height;
  stream >> width
         >> height;
  m_grid.set_dimensions(width, height);

  std::vector<Tile> tiles;
  tiles.reserve(static_cast<size_t>(width) * height);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      auto& tile = tiles.emplace_back();
      tile.x = x;
      tile.y = y;
    }
  }
  m_grid.set_tiles(std::move(tiles));
}

void Game::turn_input(std::istream& stream) {
  stream >> m_me.matter
         >> m_opp.matter;
  static std::vector<Tile> tiles(m_grid.height() * m_grid.width());
  for (auto y = 0; y < m_grid.height(); ++y) {
    for (auto x = 0; x < m_grid.width(); ++x) {
      stream >> tiles[m_grid.index_of(x, y)];
    }
  }
  m_grid.swap_tiles(tiles);
}

} // namespace kog
