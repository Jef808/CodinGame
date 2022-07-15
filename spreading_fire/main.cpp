#include <iostream>
#include <string>
#include <vector>

#include "agent.h"
#include "game.h"
#include "search.h"


inline Move make_move(size_t index) {
  return { Move::Type::Cut, index };
}


int main(int argc, char *argv[]) {
  constexpr Move wait{ Move::Type::Wait, NULL_INDEX };

  Game game;
  game.init_input(std::cin);

  std::cerr << "Tree fire duration: " << game.duration_fire_tree() << std::endl;
  std::cerr << "Fire origin 'fire_progress': " << game.fire_progress()[game.fire_origin()] << std::endl;
  Agent agent(game);

  search::init(game, agent);
  std::vector<Move> moves = search::get_moves(game);
  size_t m = 0;

  while (true) {
    game.turn_input(std::cin);
    Move move = m < moves.size() && game.is_ready()? moves[m++]: WAIT;

    std::cout << game.format_move(move) << std::endl;

    std::cerr << "Fire origin 'fire_progress': " << game.fire_progress()[game.fire_origin()] << std::endl;
    game.apply(move);
  }

  return EXIT_SUCCESS;
}
