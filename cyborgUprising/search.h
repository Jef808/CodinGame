#ifndef SEARCH_H_
#define SEARCH_H_

#include "game.h"

#include <deque>

namespace cyborg::search {

void init(const Game& game);

Action search(const Game& game);

/**
 * Attack from our biggest node
 * to its closest target.
 */
Action attack_greedy(const Game& game);

} // namespace cyborg::search

#endif // SEARCH_H_
