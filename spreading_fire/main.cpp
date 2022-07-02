#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "agent.h"
#include "game.h"

int main(int argc, char *argv[]) {
  Game game;
  game.init_input(std::cin);
  Agent agent(game);

  while (true) {
    game.turn_input(std::cin);
    // bool okay = game.test_against_turn_input();
    // assert(okay);

    Move move = agent.choose_move();

    if (move.type == Move::Type::Wait) {
      std::cout << "WAIT" << std::endl;
    } else {
      auto [x, y] = game.coords(move.index);
      std::cout << x << ' ' << y << std::endl;
    }

    game.apply(move);
  }

  return EXIT_SUCCESS;
}
