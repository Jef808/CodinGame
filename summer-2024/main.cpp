#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

#include "olymbits.h"
#include "agent.h"

using namespace olymbits;

inline std::ostream &operator<<(std::ostream &_out, Action a) {
  switch (a) {
  case Action::LEFT:
    return _out << "LEFT";
  case Action::DOWN:
    return _out << "DOWN";
  case Action::RIGHT:
    return _out << "RIGHT";
  case Action::UP:
    return _out << "UP";
  default:
    return _out << "UP";
  }
}

int main() {
  Olymbits game;
  game.init(std::cin);

  while (true) {
    game.turn_init(std::cin);

    auto action_weights = get_action_weights(game);

    std::array<int, 4> order {0, 1, 2, 3};
    for (int i = 0; i < 4; ++i) {
      std::cerr << action_weights[i] << ' ';
    }
    std::cerr << std::endl;

    std::sort(order.begin(), order.end(), [&](int a, int b) {
      return action_weights[a] > action_weights[b];
    });

    std::cout << Action(order[0]) << std::endl;
  }
}
