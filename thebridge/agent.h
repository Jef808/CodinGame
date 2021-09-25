#ifndef AGENT_H_
#define AGENT_H_

#include "tb.h"
#include "types.h"

namespace tb {

class Agent {
public:
    Agent(Game& g) :
        game{g} { }

    void set_time_lim(int tl) { time_limit = tl; }
    Action get_next();

private:
    Game& game;

    int depth_completed;
    int time_limit;
};


} // namespace tb

#endif // AGENT_H_
