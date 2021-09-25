#include "agent.h"
#include "tb.h"
#include "ttable.h"

#include <chrono>
#include <deque>

namespace tb {

namespace {

    std::deque<State> history;


    struct TimeUtil {
        typedef std::chrono::steady_clock Clock;
        typedef Clock::time_point Point;

        TimeUtil() = default;
        void set_limit(int lim_ms) { limit = lim_ms; }
        void start() { m_start = Clock::now(); }
        bool out() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - m_start).count()
                < limit;
        }

        Point m_start;
        int limit;
    };

    TimeUtil Time;

    struct RootAction
    {
        RootAction(Action a = Action::None) :
            action{a}
        {}
        Action action;
        Value value{ -Value::Infinite };
    };

    typedef std::vector<RootAction> RootActions;

    Value eval(Game& g, const Action a, int depth = 0);

} // namespace



Action Agent::get_next() {

    const auto& actions = game.valid_actions2();
    RootActions ractions;

    for (auto a : actions) {
        ractions.push_back(RootAction{a});
    }

    Action best_action = Action::None;
    Value best_value = -Value::Infinite;

    for (auto& ra : ractions) {
        ra.value = eval(game, ra.action);
        if (ra.value > best_value) {
            best_action = ra.action;
        }
    }

    return best_action;
}

namespace {

    /**
     * Do an iterative search so that we can output a decent
     * solution in time even if we don't solve the game
     * before the turn time limit.
     */
    Value eval(Game& g, const Action a, int depth)
    {
        Value val;
        return val;
    }


}



} //namespace tb
