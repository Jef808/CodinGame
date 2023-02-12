#include <cstddef>
#include <iostream>
#include <vector>

#include "world.h"

World::World() {
    std::cin >> m_width >> m_height;
    std::cin.ignore();
    m_tiles.resize(static_cast<std::vector<int>::size_type>(m_width) * m_height);
}

const Tile& World::update_tile(int row, int col) {
    Tile& tile = m_tiles[get_idx(row, col)];

    std::cin >> tile.scrap_amount >> tile.owner >> tile.units >> tile.recycler
            >> tile.can_build >> tile.can_spawn >> tile.in_range_of_recycler;
    std::cin.ignore();

    return tile;
}

std::ostream& Tile::dump(std::ostream& ioOut) const {
    ioOut << x << " " << y;
    return ioOut;
}
