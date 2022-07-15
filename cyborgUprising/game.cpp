#include "game.h"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace cyborg {

void Game::init(std::istream &is) {
  int factoryCount; // the number of factories
  is >> factoryCount;
  is.ignore();

  m_nodes.reserve(factoryCount);
  std::generate_n(std::back_inserter(m_nodes), factoryCount,
                  [id = -1]() mutable {
                    return Node{
                        ++id,
                    };
                  });
  m_edges.resize(factoryCount, std::vector<Edge>(factoryCount));

  int linkCount; // the number of links between factories
  is >> linkCount;
  is.ignore();
  for (int i = 0; i < linkCount; i++) {
    int factory1;
    int factory2;
    int distance;
    is >> factory1 >> factory2 >> distance;
    is.ignore();
    m_edges[factory1][factory2].distance = distance;
    m_edges[factory2][factory1].distance = distance;
  }

  m_bombs.reserve(4);
}

void Game::reset() {
  std::for_each(m_nodes.begin(), m_nodes.end(), [](Node &node) {
    node.owner = Owner::None;
    node.n_troops = 0;
    node.production = 0;
  });
  std::for_each(m_edges.begin(), m_edges.end(), [](std::vector<Edge> &edges) {
    std::for_each(edges.begin(), edges.end(), [](Edge &edge) {
      edge.n_troops = 0;
      edge.troops_owner = Owner::None;
      edge.turns_until_arrival = INT_INFTY;
    });
  });
}

void Game::turn_update(std::istream &is) {
  using std::string;
  reset();
  int entityCount; // the number of entities (e.g. factories and troops)
  is >> entityCount;
  is.ignore();

  for (int i = 0; i < entityCount; i++) {
    int entityId;
    string entityType;
    int arg1;
    int arg2;
    int arg3;
    int arg4;
    int arg5;
    is >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4 >> arg5;
    is.ignore();
    switch (entityType[0]) {
    case 'F': {
      Node &node = m_nodes[entityId];
      node.owner = arg1 == -1  ? Owner::Them
                   : arg1 == 0 ? Owner::None
                               : Owner::Me;
      node.n_troops = arg2;
      node.production = arg3;
      break;
    }
    case 'T': {
      Edge &edge = m_edges[arg2][arg3];
      edge.troops_owner = arg1 == -1 ? Owner::Them : Owner::Me;
      edge.n_troops = arg4;
      edge.turns_until_arrival = arg5;
      break;
    }
    case 'B': {
      auto it = std::find_if(
          m_bombs.begin(), m_bombs.end(),
          [entityId](const auto &bomb) { return bomb.id == entityId; });
      // Update new bombs only
      if (it == m_bombs.end()) {
        Bomb &bomb = m_bombs.emplace_back();
        bomb.id = entityId;
        bomb.owner = arg1 == -1 ? Owner::Them : Owner::Me;
        bomb.source = arg2;
        if (arg3 != -1) {
          bomb.maybe_targets.push_back(arg3);
        } else {
          std::for_each(m_edges[bomb.source].begin(),
                        m_edges[bomb.source].end(),
                        [target = -1, distance = arg4,
                         ins = std::back_inserter(bomb.maybe_targets)](
                            const Edge &edge) mutable {
                          if (edge.distance == distance)
                            ins = ++target;
                        });
        }
      }
      break;
    }
    default: {
      throw std::runtime_error("Invalid entity type.");
    }
    }
  }
}

std::ostream &operator<<(std::ostream &out, const Action &action) {
  out << "WAIT";
  if (action.move != std::nullopt) {
    out << ";MOVE" << ' ' << action.move->source << ' ' << action.move->target
        << ' ' << action.move->n_troops;
  }
  return out;
}

} // namespace cyborg
