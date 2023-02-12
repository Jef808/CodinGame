#include "worldinfo.h"

#include <cstddef>
#include <iostream>

#include "world.h"

using namespace std;
using size_type = std::vector<std::reference_wrapper<const Tile>>::size_type;

WorldInfo::WorldInfo(World& world)
    : m_world{world}
{
  m_my_tiles.reserve(static_cast<size_type>(world.width()) * world.height());
  m_opp_tiles.reserve(static_cast<size_type>(world.width()) * world.height());
  m_neutral_tiles.reserve(static_cast<size_type>(world.width()) * world.height());
  m_my_units.reserve(static_cast<size_type>(world.width()) * world.height());
  m_opp_units.reserve(static_cast<size_type>(world.width()) * world.height());
  m_my_recyclers.reserve(static_cast<size_type>(world.width()) * world.height());
  m_opp_recyclers.reserve(static_cast<size_type>(world.width()) * world.height());
}

void WorldInfo::reset_data() {
    m_my_tiles.clear();
    m_opp_tiles.clear();
    m_neutral_tiles.clear();
    m_my_units.clear();
    m_opp_units.clear();
    m_my_recyclers.clear();
    m_opp_recyclers.clear();
}

void WorldInfo::update() {
    reset_data();
    cin >> m_my_matter >> m_opp_matter;
    cin.ignore();
    for (int y = 0; y < m_world.height(); ++y) {
        for (int x = 0; x < m_world.width(); ++x) {
          const Tile& tile = m_world.update_tile(x, y);
          if (tile.owner == ME) {
            cerr << "That's my tile" << endl;
                m_my_tiles.emplace_back(tile);
                if (tile.units > 0) {
                    m_my_units.emplace_back(tile);
                } else if (tile.recycler) {
                    m_my_recyclers.emplace_back(tile);
                }
            } else if (tile.owner == OPP) {
                cerr << "That's their tile" << endl;
                m_opp_tiles.emplace_back(tile);
                if (tile.units > 0) {
                    m_opp_units.emplace_back(tile);
                } else if (tile.recycler) {
                    m_opp_recyclers.emplace_back(tile);
                }
            } else {
              cerr << "That's a neutral tile" << endl;
                m_neutral_tiles.emplace_back(tile);
            }
        }
    }
}
