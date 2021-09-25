#include "agent.h"
#include "tb.h"
#include "ttable.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <deque>

namespace tb {

namespace {

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

    std::deque<State> StateHistory;
    std::deque<Action> ActionHistory;

} // namespace


// If we used "Speed" or "Slow", we shouldn't use the other one
// before "Jump", "Up" or "Down"
Action excluded() {
    Action ret = Action::None;
    if (ActionHistory.empty())
        return ret;

    auto q = ActionHistory.rbegin();
    for (; q != ActionHistory.rend(); --q) {
        if (*q == Action::Speed)
            return Action::Slow;
        if (*q == Action::Slow)
            return Action::Speed;
        if (*q == Action::Wait)
            continue;
        return Action::None;
    }
    return Action::None;
}

Value Score(Game& g) {
    if (g.is_lost())
        return Value::Known_loss;
    if (g.is_won())
        return Value::Known_win;
    return Value(500 * (g.pos() / g.turn()+1 - g.road_length() / 50)
                 + g.ratio_bikes_left());
}

Action Agent::get_next() {

    const auto& actions = game.candidates();
    RootActions ractions;

    if (actions.empty()) return Action::None;

    for (auto a : actions) {
        if (a == excluded())
            continue;
        ractions.push_back(RootAction{a});
    }

    RootAction best_raction = RootAction{Action::None};
    Value best_value = -Value::Infinite;

    for (int i=0; i<10; ++i) {
        int depth = depth_completed + 1;

        for (auto& ra : ractions) {
            if (ra.value == Value::Known_loss)
                continue;

            ra.value = eval(game, ra.action, depth);

            if (ra.value > best_value) {
                best_raction = ra;
                best_value = ra.value;
            }
        }

        ++depth_completed;
        std::swap(ractions[0], best_raction);
        if (ractions[0].value == Value::Known_win)
            break;
    }

    Action ret = ractions[0].action;
    ActionHistory.push_back(ret);
    State& st = StateHistory.emplace_back();
    game.apply(ret, st);
    return ret;
}

namespace {

    /**
     * Do an iterative search so that we can output a decent
     * solution in time even if we don't solve the game
     * before the turn time limit.
     */
    Value eval(Game& g, const Action a, int depth)
    {
        State st;
        g.apply(a, st);


        auto [win, loss] = std::make_pair(g.is_won(), g.is_lost());
        if (win) {
            Value ret = Value::Known_win;
            g.undo();
            return ret;
        } else if (loss) {
            Value ret = Value::Known_loss;
            g.undo();
            return ret;
        }

        if (depth == 0) {
            Value ret = Score(g);
            g.undo();
            return ret;
        }

        // Didn't need to record the action for depth 0.
        ActionHistory.push_back(a);

        // Need to allocate space for this outside of the function
        std::array<Action, Max_actions> actions;
        std::fill(actions.begin(), actions.end(), Action::None);

        const auto& cands = g.candidates();
        std::copy(cands.begin(), cands.end(), actions.begin());

        // Exclude the bad choice
        auto it = std::find(actions.begin(), actions.end(), excluded());
        *it = Action::None;

        if (cands.size() == 1 && cands[0] == excluded()) {
            Value val = Value(-5000);
            g.undo();
            ActionHistory.pop_back();
            return val;
        }

        Value best_value = -Value::Infinite;
        Action best_action = Action::None;
        for (auto a : actions) {
            if (a == Action::None)
                continue;
            Value val = eval(g, a, depth - 1);
            if (val == Value::Known_loss)
                continue;
            if (val == Value::Known_win) {
                best_value = val;
                break;
            }
            if (val > best_value) {
                best_value = val;
                best_action = a;
            }
        }
        g.undo();
        ActionHistory.pop_back();
        return best_value;
    }
}



} //namespace tb
