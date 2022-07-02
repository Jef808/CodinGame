#include "distance.h"
#include "game.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <limits>
#include <set>

namespace {
constexpr int INFTY = std::numeric_limits<int>::infinity();
} // namespace

GridDistance::GridDistance(const Game& game)
  : m_game{game}, m_dist{}, m_parent{}
{

}

void GridDistance::set_source(const size_t source) {
  m_dist.resize(m_game.size(), INFTY);
  std::fill(m_dist.begin(), m_dist.end(), INFTY);

  m_parent.resize(m_game.size(), 0);
  std::fill(m_parent.begin(), m_parent.end(), INFTY);

  m_source = source;
  m_dist[source] = 0;
  m_parent[source] = source;

  std::deque<size_t> q(1, source);
  std::set<size_t> seen{source};

    // Extend layer by layer
    while (not q.empty()) {

      size_t current = q.front();
      int distance = m_dist[current];
      q.pop_front();

      for (auto cur_nbh : {current - m_game.width(), current - 1, current + 1, current + m_game.width()}) {

        if (not seen.count(cur_nbh)) {
          continue;
        }

        m_dist[cur_nbh] = distance + 1;

        // if (prev_d == INFTY) {
        //   p.push_back(cur_nbh);
        // }

        // if (distance + 1 < prev_d) {
        //   prev_d = distance + 1;
        //   m_parent[cur_nbh] = current;
        // }
      }
    }
}

// void GridDistance::reset_buffers() {
//   m_dist.clear();
//   m_parent.clear();


std::vector<size_t> GridDistance::get_path(size_t target) {
  std::vector<size_t> path;
  path.clear();

  while (path.emplace_back(target) != m_source) {
    target = m_parent[target];
  }
  return path;
}
