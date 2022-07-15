#include "agent.h"
#include "game.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <limits>
#include <random>
#include <vector>


namespace {
static constexpr int INFTY = 32000;
}

Agent::Agent(Game &game)
  : m_game{game}
  , Duration{ Tree.DurationFire, House.DurationFire,
              Tree.DurationCut, House.DurationCut }
  , Value{ Tree.Value, House.Value }
  , m_distances(game.size(), INFTY)
  , m_parents(game.size())
{
  std::for_each(m_parents.begin(), m_parents.end(),
    [](auto& p){
      for (auto i : {EAST, NORTH, WEST, SOUTH}) { p[i] = INFTY; }
    });
}


Move Agent::choose_move() {
  if (m_game.m_cooldown > 0) { return {Move::Type::Wait, NULL_INDEX}; }

  m_game.get_bdry();
  size_t n = m_game.m_outer_bdry.size();

  if (n == 0) {
    return choose_random_move();
  }

  return { Move::Type::Cut, m_game.m_outer_bdry[rand() % n] };
  // if (move.index == NULL_INDEX) {


  // if (move.index == NULL_INDEX)  { return move; }

  // move.type = Move::Type::Cut;
  // return move;
}


Move Agent::choose_random_move() const {
  static std::random_device rd;
  static std::mt19937 eng{ rd() };
  static std::vector<size_t> cands;
  //Move move{ Move::Type::Wait, NULL_INDEX };

  // if (m_game.m_cooldown > 0) {
  //   return move;
  // }

  //const auto& candidates = m_game.m_outer_bdry;

  if (m_game.m_cooldown > 0) { return {Move::Type::Wait, NULL_INDEX}; }

  cands.clear();
  m_game.get_bdry();
  size_t n = m_game.m_outer_bdry.size();

  for (size_t c = 0; c < m_game.size(); ++c) {
    if (m_game.is_flammable(c)) {
      cands.push_back(c);
    }
  }
  if (cands.empty()) {
    assert(false);
  }

  std::uniform_int_distribution<size_t>
  dist{ 0, cands.size()-1 };

  return { Move::Type::Cut, cands[dist(eng)] };
}


size_t Agent::get_neighbour(size_t n, Direction d) const {
  switch(d) {
    case EAST:
    case WEST:  return n + d;
    case NORTH:
    case SOUTH: return n + m_game.width() * d / 2;
    default: throw std::runtime_error("Invalid direction");
  }
}



namespace {

class EdgeCost {
  const Game& game;
public:
  static constexpr int Infinite = INFTY;

  EdgeCost(const Game& _game)
    : game{_game}
  {}
  auto operator()(size_t source, size_t target) {
    if (not game.is_flammable(target)) {
      return Infinite;
    }
    assert(source != target);// if (source == target) {
    //   return Zero;
    // }
    return game.duration_fire(source);
  }
};

} // namespace


void Agent::generate_distance_map() {
  EdgeCost edge_cost{m_game};

  std::deque<size_t> boundary;
  std::vector<int> seen(m_game.size(), 0);

  m_distances[m_game.fire_origin()] = 0;
  boundary.push_back(m_game.fire_origin());

  while (not boundary.empty())
  {
    size_t current = boundary.front();
    boundary.pop_front();
    const int distance = m_distances[current];

    for (Direction d : {EAST, NORTH, WEST, SOUTH})
    {
      const size_t nbh = get_neighbour(current, d);

      // Avoid interior points
      if (seen[nbh] > 0) {
        continue;
      }

      const int this_distance = distance + edge_cost(current, nbh);

      // Mark points which are not flammable as seen and proceed to next nbh
      if (this_distance >= EdgeCost::Infinite) {
        seen[nbh] = 1;
        continue;
      }

      // Otherwise, add nbh to the end of the queue and update stats
      boundary.push_back(nbh);

      m_parents[nbh][-d] = this_distance;
      if (this_distance < m_distances[nbh]) {
        m_distances[nbh] = this_distance;
      }
    }

    // Add to seen set once a square's four neighbours are processed
    seen[current] = 1;
  }
}
