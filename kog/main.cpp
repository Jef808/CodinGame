#include <iostream>

#include "game.h"
#include "agent.h"

using namespace kog;

int main(int argc, char *argv[]) {
  Game game;
  game.initial_input(std::cin);

  Agent agent{game};

  for (;;) {
    game.turn_input(std::cin);

    agent.compute_turn_info();
    agent.debug(std::cerr);

    agent.choose_actions(std::cout);
  }

  return 0;
}

