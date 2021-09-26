#ifndef AGENT_H_
#define AGENT_H_

#include "tb.h"
#include "types.h"

#include <memory>

namespace Search {

struct Stack {
    tb::Action* best;
    int depth;
    int depth_best;
    tb::Action cur_action;
    tb::Value val_best;
    int action_count;
};
} // namespace Search

namespace tb {

class Agent {

    struct RootAction
    {
        // Noting the root's depth is much better than moving the content
        // of the the best_line every time we make a root move!
        explicit RootAction(Action a) :
            best_line(1, a)
        {}
        Value value{ -Value::Infinite };
        int depth_completed;
        std::vector<Action> best_line;
        bool operator==(const RootAction& ra) {
            return best_line[0] == ra.best_line[0];
        }
        // To sort the root actions in decreasing order of their value.
        bool operator<(const RootAction& ra) {
            return ra.value > value;
        }
    };

    typedef std::vector<RootAction> RootActions;

public:
    Agent(Game& g) :
        game{g} { }

    void init(size_t pow2 = 0);
    void set_time_lim(int tl) { time_limit = tl; }
    Action get_next();

private:
    Game& game;
    RootActions ractions;
    int time_limit;
    int depth_root;
    int depth_completed;
    std::array<Search::Stack, Max_depth + 1> stack{};
    Search::Stack* ss = &stack[1];
    std::array<State, Max_depth + 1> states{};
    State* st = &states[1];
    bool found_winner { false };

    std::unique_ptr<RootAction> ra_backup;

    void play_rootaction(Action a);
    void play_rootaction2();
};


} // namespace tb

#endif // AGENT_H_
