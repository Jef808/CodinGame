#ifndef AGENT_H_
#define AGENT_H_

#include "tb.h"

namespace tb {

struct ExtAction;

class Agent {
public:
    using Cost = int;

    Agent() = default;
    void solve(const Game& game, int time_limit_ms = 0);

private:
    Game game;
    int time_limit_ms { 0 };

    void init(const Game& game, int time_limit_ms);
    const std::vector<ExtAction>& generate_actions(const State& s) const;

    Cost depth_first_search(const State&, int depth, bool& timeout) const;
};

} // namespace tb

#endif // AGENT_H_
