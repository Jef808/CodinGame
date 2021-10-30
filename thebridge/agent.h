#ifndef AGENT_H_
#define AGENT_H_

#include "tb.h"

namespace tb {

struct ExtAction;
struct Stack;

class Agent {
public:
    using Cost = int;

    Agent() = default;
    void solve(const Game& game, int time_limit_ms = 0);

    Action next_action() const;

private:
    Game game;
    int time_limit_ms { 0 };
    int depth_searched{ 0 };
    mutable int actions_out_count{ 0 };

    void init(const Game& game, int time_limit_ms);
    const std::vector<ExtAction>& generate_actions(const State& s) const;

    Cost depth_first_search(int depth, bool& timeout, Stack*) const;
};

} // namespace tb

#endif // AGENT_H_
