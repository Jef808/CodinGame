#include "game.h"

#include <iostream>

namespace kog {

Game::Game(Grid&& grid)
  : m_grid{grid}
{
}

void Game::initial_input(std::istream& stream) {
  int width = 0;
  int height = 0;
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

  static std::vector<Tile> tiles = [W=m_grid.width(),H=m_grid.height()]{
    std::vector<Tile> _tiles(W*H);
    for (auto y = 0; y < H; ++y) {
      for (auto x = 0; x < W; ++x) {
        auto& tile = tiles[x+y*W];
        tile.x = x;
        tile.y = y;
      }
    }
    return _tiles;
  }();

  for (auto y = 0; y < m_grid.height(); ++y) {
    for (auto x = 0; x < m_grid.width(); ++x) {
      stream >> tiles[m_grid.index_of(x, y)];
    }
  }
  m_grid.swap_tiles(tiles);
}

void Game::output_grid(std::ostream& stream) {
  const auto width = m_grid.width();
  const auto height = m_grid.height();

  for (auto y = 0; y < height; ++y) {
    for (auto x = 0; x < width; ++x) {
      if (m_grid.at(m_grid.index_of(x, y)).is_blocked()) {
        stream << 'B';
      } else {
        stream << 'F';
      }
      if (x < width - 1) {
          stream << ' ';
      }
    }
    stream << '\n';
  }
}

} // namespace kog
