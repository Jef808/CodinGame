#ifndef AGENT_H_
#define AGENT_H_

#include "dp.h"

namespace dp {

class Agent {
public:
    explicit Agent(Game& game);
    std::string best_choice();
private:
    Game& game;
};

inline Agent::Agent(Game& _game) :
    game(_game)
{
}

inline std::string Agent::best_choice() {
    return "WAIT";
}


} // namespace dp

#endif // AGENT_H_
