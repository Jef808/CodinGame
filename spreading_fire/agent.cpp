#include "agent.h"
#include "distance.h"

#include <algorithm>
#include <random>
#include <iostream>
#include <vector>

Agent::Agent(Game &game)
  : m_game{game}
  , house_distance{game}
  , fire_expand{game}
  , O{ game.fire_origin }
  , m_houses{}
{

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


void Agent::collect_houses_data() {
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


void Agent::collect_distances_data() {
  // Compute the

  fire_expand.set_source(m_game.index(O.x, O.y));

  // Get max number of rounds (before the fire expands to
  // all the grid)
  size_t T = 0;

  if (m_game.m_width > m_game.m_height) {
    for (auto j = 0; j < m_game.m_height; ++j)
      for (auto i : { 0U, (unsigned)(m_game.m_width - 1) }) {
        auto d = fire_expand.get_distance(m_game.m_width * j + i);
        if (d > T) { T = d; }
    }
  }
  else {
    for (auto i = 0; i < m_game.m_width; ++i)
      for (auto j : { 0U, (unsigned)(m_game.m_height - 1) }) {
        auto d = fire_expand.get_distance(m_game.m_width * j + i);
        if (d > T) { T = d; }
    }
  }

  // Compute the value destroyed if let fire is let alone by distance
  // from the start (corresponding to each expansion)
  std::vector<int> value_burned(T, 0);

  for (size_t n = 0; n < m_game.size(); ++n) {
    size_t t = fire_expand.get_distance(n);
    value_burned[t] += m_game.value(n);
  }


}
