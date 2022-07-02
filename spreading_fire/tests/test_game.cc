#include "game.h"
#include "agent.h"

#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {

    std::ifstream ifs { TEST_DATA_DIR "/input1.txt" };

    if (not ifs) {
        std::cerr << "Failed to open input file" << std::endl;
    }

  Game game{};

  std::cerr << "Created a Game" << std::endl;

  game.init_input(ifs);

  std::cerr << "Initialized Game" << std::endl;

  while (true) {
    //game.turn_input(std::cin);
    // bool okay = game.test_against_turn_input();
    // assert(okay);

    Move move = {Move::Type::Wait, NULL_INDEX};//agent.choose_random_move();

    std::cerr << "Got Move" << std::endl;

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
