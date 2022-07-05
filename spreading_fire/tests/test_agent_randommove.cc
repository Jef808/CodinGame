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

  Agent agent{game};
  std::cerr << "Initialized Agent" << std::endl;

  while (true) {
    //game.turn_input(std::cin);
    // bool okay = game.test_against_turn_input();
    // assert(okay);

    Move move = agent.choose_move();

    std::cerr << "Got Move" << std::endl;

    if (move.type == Move::Type::Wait) {
      std::cout << "WAIT" << std::endl;
    } else {
      auto [x, y] = game.coords(move.index);
      std::cout << x << ' ' << y << std::endl;
    }

    game.apply(move);

    bool game_over = true;
    for (size_t i = 0; i < game.size(); ++i) {
      if (game.is_flammable(i)) {
        game_over = false;
        break;
      }
    }

    if (game_over) {
      std::cerr << "GAME OVER" << std::endl;
      break;
    }
  }



    return EXIT_SUCCESS;
}
