#include "game.h"
#include "search.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

using namespace cyborg;

std::ostream& operator<<(std::ostream& out, const std::vector<Action>& vec) {
  auto last = vec.end() - 1;
  for (auto it = vec.begin(); it < last; ++it) {
    out << *it << ';';
  }
  return out << *last;
}


int main() {
  using std::cin;
  using std::cout;
  using std::endl;

  Game game;
  game.init(cin);

  // game loop
  while (1) {
    game.turn_update(cin);

    const std::vector<Action>& actions = search::attack_greedy(game);
    cout << actions << endl;
  }
}
