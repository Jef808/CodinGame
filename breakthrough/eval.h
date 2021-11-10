#ifndef __EVAL_H_
#define __EVAL_H_

#include <string>

#include "types.h"

class Game;

namespace Eval {

int evaluate(const Game& game);

std::pair<int, std::string> trace(const Game& game);

} // namespace Eval

#endif
