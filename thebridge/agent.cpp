#include "agent.h"
#include "tb.h"
#include "timeutil.h"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace {

    using Cost = tb::Agent::Cost;

    namespace cost {
        enum : Cost { zero = 0,
               known_win = 1,
               unknown = tb::Max_depth / 2,
               known_loss = tb::Max_depth + 1,
               max = tb::Max_depth + 2,
             };
    }

}  // namespace

namespace tb {

    struct ExtAction {
        ExtAction()
            : action { Action::None }
            , cost { cost::unknown }
        {
        }
        explicit ExtAction(Action a)
            : action { a }
            , cost { a == Action::None ? cost::max : cost::unknown }
        {
        }
        operator Action() const { return action; }
        Action action;
        Cost cost;
        /// Sorting orders actions in increasing order of cost
        inline bool operator<(const ExtAction& other) const
        {
            return cost < other.cost;
        }
        inline bool operator==(const ExtAction& other) const
        {
            return action == other.action;
        }
    };

namespace {

    /// Globals
    using ActionList = std::array<ExtAction, Max_actions>;

    const Params* prm;
    Timer time;

    ActionList sactions[Max_depth + 1];
    std::array<State, Max_depth + 1> states;
    State* ps { &states[0] };

} // namespace


/**
 * Main method to be called from main.
 */
void Agent::solve(const Game& game, int time_limit_ms)
{
    time.reset();
    bool timeout = false;

    init(game, time_limit_ms);

    Cost best_cost = cost::max;

    ActionList* sa = &sactions[0];

    for (int depth = 0; depth < Max_depth; ++depth)
    {
        best_cost = depth_first_search(states[0], depth, timeout);

            /// The stable version of the sort algorithm guarantees to preserve
            /// the ordering of two elements of equal value.
            std::stable_sort(sa->begin(), sa->end());

            if (best_cost <= cost::known_win)
                return;

            if (timeout) {
                continue;
            }

            break;

        /// Check if we have time for the next depth really
        if (time.out()) {
            return;
            time.reset();
        }
    }
}

void Agent::init(const Game& game, int time_limit_ms)
{
    prm = game.parameters();
    states[0] = *game.state();

    this->time_limit_ms = time_limit_ms;

    std::for_each(&sactions[0], &sactions[Max_depth], [](auto& al){
        al.fill(ExtAction{});
    });
}

namespace {


inline size_t road_length()
{
    return prm->road[0].size();
}

/**
 * The greatest lower bound for the number of turns in
 * which it is possible to reach the end of the road.
 */
inline Cost future_cost_lb(const State& s)
{
    int p = s.pos, v = s.speed, t = 0;

    while (p < road_length()) {
        t += 1;
        p += p + v + 1;
    }

    return t;
}

bool inline n_bikes(const State& s)
{
    return std::count_if(s.bikes.begin(), s.bikes.end(), [](auto b){ return b; });
}

bool inline at_eor(const State& s)
{
    return s.pos >= prm->road[0].size();
}

bool inline is_lost(const State& s)
{
    return future_cost_lb(s) > Max_depth - s.turn
        || n_bikes(s) < prm->min_bikes;
}

} // namespace


const std::vector<ExtAction>& Agent::generate_actions(const State& s) const
{
    static std::vector<ExtAction> ret;
    ret.clear();

    const auto& actions = game.valid_actions(s);
    std::transform(actions.begin(), actions.end(), std::back_inserter(ret), [](auto a) {
        return ExtAction(a);
    });

    return ret;
}


Cost Agent::depth_first_search(const State& s, int depth, bool& timeout) const
{
    if (at_eor(s))
        return cost::known_win;

    std::array<ExtAction, Max_actions> actions {
        ExtAction(Action::None),
        ExtAction(Action::None),
        ExtAction(Action::None),
        ExtAction(Action::None),
        ExtAction(Action::None)
    };

    const auto& candidates = generate_actions(s);
    std::copy(candidates.begin(), candidates.end(), actions.begin());

    Cost best_cost = cost::max;
    Action best_action = Action::None;

    if (depth == 0)
    {
        for (auto& a : actions) {
            if (a == Action::None || a.cost == cost::known_loss)
                continue;

            State& st = game.apply(*ps++, a);
            /// Check for lost before win so that we can skip some checks
            /// when checking for a win
            if (is_lost(st)) {
                return cost::known_loss;
            }
            if (at_eor(st)) {
                return cost::known_win;
            }
        }

        return best_cost == cost::max ? cost::known_loss : best_cost;
    }

    // [[ depth > 0 ]]
    // Sort the actions here

    for (auto& a : actions) {
        if (a == Action::None || a.cost == cost::known_loss)  // a.cost >= cost::known_loss
            continue;

        State& st = game.apply(*ps++, a);
        a.cost = depth_first_search(s, depth - 1, timeout);
        --ps;

        if (a.cost == cost::known_win) {
            // Immediately return down to solve() and
            // reconstruct the winning action sequence

            best_cost = a.cost;
            best_action = a;
            break;
        }
        if (a.cost < best_cost) {
            best_cost = a.cost;
            best_action = a;
        }
    }

    return best_cost;
}

} // namespace tb
