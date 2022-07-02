#include "agent.h"
#include "distance.h"

#include <algorithm>
#include <random>
#include <iostream>
#include <vector>

Agent::Agent(Game &game)
  : m_game{game}
  , house_distance{game.width(), game.height()}
  , fire_expand{game.width(), game.height()}
  , O{ game.fire_origin }
  , m_houses{}
{

}

void Agent::init() {

  for (const auto& c : m_game.m_cells) {
    if (c.type() == Cell::Type::House)
      m_houses.emplace_back(c.index());
  }

  std::sort(m_houses.begin(), m_houses.end(),
            [&](auto a, auto b) {
              return manhattan_distance(O, m_game.coords(a)) <
                     manhattan_distance(O, m_game.coords(b));
            });

  house_distance.set_source(m_houses.front());

  fire_expand.set_source(m_game.index(O.x, O.y));
}



Move Agent::choose_move() {
  Move move {Move::Type::Wait, NULL_INDEX};

  if (m_game.m_cooldown > 0) {
    return move;
  }

  m_game.get_bdry();

  move.index = get_point_towards_houses();

  if (move.index == NULL_INDEX) {

    move = choose_random_move();

    if (move.index == NULL_INDEX)  {
      return move;
    }
  }

  move.type = Move::Type::Cut;
  return move;
}



size_t Agent::get_point_towards_houses() {
  size_t ret = NULL_INDEX;

  if (m_houses.empty() || m_game.m_outer_bdry.empty()) {
    return ret;
  }

  // Sort the cells past the fire horizon wrt to their distance
  // to the first house.
  house_distance.distance_sort(m_game.m_outer_bdry.begin(), m_game.m_outer_bdry.end());

  return m_game.m_outer_bdry.front();
}


Move Agent::choose_random_move() {
  static std::random_device rd;
  static std::mt19937 eng{ rd() };

  //static std::vector<size_t> candidates;

  Move ret{ Move::Type::Wait, NULL_INDEX };

  if (m_game.m_cooldown > 0) {
    return ret;
  }

  const auto& candidates = m_game.m_outer_bdry;


  if (candidates.empty()) {
    return ret;
  }

  std::uniform_int_distribution<size_t> dist(0, candidates.size()-1);
  return { Move::Type::Cut, candidates[dist(eng)] };
}
