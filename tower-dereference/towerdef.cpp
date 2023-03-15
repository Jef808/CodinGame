#include "towerdef.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

namespace TowerDefense {

Game initialize(std::istream& is) {
  Game game;
  int player_id { -1 };
  is >> player_id; is.ignore();
  if (player_id == 0) {
    game.m_player_id = Player::LEFT;
  }
  else if (player_id == 1) {
    game.m_player_id = Player::RIGHT;
  }
  else {
    throw std::runtime_error("Game::initialize: Invalid player_id: " + std::to_string(player_id));
  }

  is >> game.m_width >> game.m_height; is.ignore();

  for (auto i = 0; i < game.m_height; ++i) {
    std::string buf;
    std::getline(is, buf);
    std::transform(buf.begin(), buf.end(), std::back_inserter(game.m_grid.tiles),
                   [](char c) {
                     Grid::Tile ret;
                     switch (c) {
                       case '.': ret = Grid::Tile::Canyon; break;
                       case '#': ret = Grid::Tile::Plateau; break;
                       throw std::runtime_error("Invalid tile input: " + std::string(1, c));
                     }
                     return ret;
                   });
  }

  return game;
}

inline Player input_player(std::istream& is) {
  Player player { Player::NONE };
  int _player { -1 };
  is >> _player;
  if (_player == 0) {
    player = Player::LEFT;
  }
  else if (_player == 1) {
    player = Player::RIGHT;
  }
  else {
    throw std::runtime_error("Unknown player input: " + std::to_string(_player));
  }
  return player;
}

void Game::input(std::istream& is) {
  is >> m_my_money >> m_my_lives; is.ignore();
  is >> m_opp_money >> m_opp_lives; is.ignore();

  m_towers.clear();
  int n = -1;
  is >> n; is.ignore();
  std::string buf;
  for (auto i = 0; i < n; ++i) {
    auto& tower = m_towers.emplace_back();
    std::getline(is, buf);
    bool okay = false;

    switch (buf[0]) {
      case 'F': tower.type = Tower::Fire; break;
      case 'H': tower.type = Tower::Heal; break;
      case 'G': {
        switch (buf[1]) {
          case 'U': tower.type = Tower::Gun; okay = true; break;
          case 'L': tower.type = Tower::Glue; okay = true; break;
        }
      }
      if (!okay) {
        throw std::runtime_error("Unknown tower type: " + buf.substr(0, buf.find(' ') + 1) + "...");
      }
    }

    std::stringstream ss { buf.substr(buf.find(' ')) };

    ss >> tower.id;
    tower.owner = input_player(ss);
    ss >> tower.x
       >> tower.y
       >> tower.damage
       >> tower.range
       >> tower.reload
       >> tower.cooldown;

    m_grid.tiles[tower.x + m_width * tower.y] = (Grid::Tile)(m_grid.tiles[tower.x + m_width * tower.y] | Grid::Tile::Tower);
  }

  m_attackers.clear();
  n = -1;
  is >> n; is.ignore();
  for (auto i = 0; i < n; ++i) {
    auto& attacker = m_attackers.emplace_back();
    std::getline(is, buf);
    std::stringstream ss { buf };
    ss >> attacker.id;
    attacker.owner = input_player(ss);
    ss >> attacker.x
       >> attacker.y
       >> attacker.hp
       >> attacker.max_hp
       >> attacker.speed
       >> attacker.max_speed
       >> attacker.slow_time
       >> attacker.bounty;

    m_grid.tiles[attacker.x + m_width * attacker.y] = (Grid::Tile)(m_grid.tiles[attacker.x + m_width * attacker.y] | Grid::Tile::Attacker);
  }
}

Grid::Tile Game::at(int x, int y) const {
  if (x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1) {
    std::stringstream ss;
    ss << "(x, y) out of bounds, where x = "
       << x
       << "and y = "
       << y;
    throw std::runtime_error(ss.str());
  }
  return m_grid.tiles[x + m_width * y];
}

Grid::Tile Game::at(const Point& p) const {
  return at(p.x, p.y);
}

} // namespace TowerDef
