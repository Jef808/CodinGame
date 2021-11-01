#ifndef AGENT_H_
#define AGENT_H_

#include "tb.h"

#include <iosfwd>

namespace tb {

class Agent {
public:

    Agent() = default;
    void init(const Game& game);
    void search(const Game& game);
    Action best_action() const;

private:
    Game game;
    mutable int actions_out_count{ 0 };

    bool depth_first_search(const State& state, int max_depth) const;
};

} // namespace tb

#endif // AGENT_H_
