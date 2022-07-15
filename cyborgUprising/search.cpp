#include "search.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <stdexcept>

namespace cyborg::search {

constexpr int MAX_MOVE_LEN = 20;
constexpr int MAX_NODES = 15;

namespace {

struct ExtNode;
std::vector<ExtNode> nodes_eval;

struct ExtNode {
  bool in_battle{false};
  bool attacked{false};
  bool defended{true};
};

struct Movement {
  int n_turns{INT_INFTY};
  int n_troops{0};
};
std::vector<Movement> TroopsNeeded;
constexpr const Movement movementNull = Movement{};

std::array<int, MAX_MOVE_LEN> incoming(const Game &game, const Node &node) {
  std::array<int, MAX_MOVE_LEN> ret{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (size_t i = 0; i < game.nodes().size(); ++i) {
    const Edge &edge = game.edges()[i][node.id];
    if (edge.turns_until_arrival < INT_INFTY) {
      ret[edge.turns_until_arrival] +=
          (edge.troops_owner == Owner::Me ? 1 : -1) * edge.n_troops;
    }
  }
  return ret;
}

} // namespace

void init(const Game &game) { nodes_eval.resize(game.nodes().size()); }

Action search(const Game &game) { return make_action<Action::Type::Wait>(); }

const std::vector<Action> &attack_greedy(const Game &game) {
  static std::vector<Action> ret;
  ret.clear();
  ret.push_back(make_action<Action::Type::Wait>());

  const Node *my_biggest_node = nullptr;
  int my_biggest_army = 0;
  for (const auto &node : game.nodes()) {
    if (node.owner == Owner::Me && node.n_troops > my_biggest_army) {
      my_biggest_node = &node;
      my_biggest_army = node.n_troops;
    }
  }

  if (my_biggest_node == nullptr) {
    return ret;
  }

  const Node *closest_target = nullptr;
  int target_distance = INT_INFTY;
  for (const auto &node : game.nodes()) {
    int distance = game.edges()[my_biggest_node->id][node.id].distance;
    if (node.owner != Owner::Me && distance < target_distance) {
      closest_target = &node;
      target_distance = distance;
    }
  }

  if (closest_target == nullptr) {
    return ret;
  }

  std::cerr << "Biggest node: " << my_biggest_node->id
            << "\nClosest target: " << closest_target->id
            << "\nn_troops: " << my_biggest_node->n_troops << std::endl;

  ret.push_back(make_action<Action::Type::Move>(my_biggest_node->id, closest_target->id,
                                         my_biggest_node->n_troops));
  return ret;
}

} // namespace cyborg::search
