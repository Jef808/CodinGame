#include <iostream>

#include "game.h"
#include "search.h"

using namespace cyborg;

int main() {
  using std::cin;
  using std::cout;
  using std::endl;

  Game game;
  game.init(cin);

  // game loop
  while (1) {
    game.turn_update(cin);

    cout << search::attack_greedy(game) << endl;
  }
}
