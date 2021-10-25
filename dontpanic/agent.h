#ifndef AGENT_H_
#define AGENT_H_

namespace dp {
class Game;
enum class Action;
}

class Agent {
public:

    Agent() = default;

    void init(const dp::Game&);

    /// The main search method
    void search();

    /// The best choice to date
    dp::Action best_choice();
};

#endif // AGENT_H_
