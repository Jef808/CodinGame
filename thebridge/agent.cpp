#include "agent.h"
#include "tb.h"
#include "timeutil.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>

namespace {

using Cost = tb::Agent::Cost;

namespace cost {
    enum : Cost { none = 0,
        known_win = 1,
        unknown = tb::Max_depth / 2,
        known_loss = tb::Max_depth + 1,
        max = tb::Max_depth + 2
    };
}

} // namespace

namespace tb {

struct ExtAction {
    ExtAction() = default;
    explicit ExtAction(Action a)
        : action { a }
        , cost { a == Action::None ? cost::max : cost::unknown }
    {
    }
    /// Implicitely convertible to an Action
    operator Action() const { return action; }
    /// Sorting orders actions in increasing order of cost
    inline bool operator<(const ExtAction& other) const {
        return cost < other.cost;
    }
    inline bool operator==(const ExtAction& other) const {
        return action == other.action;
    }
    Action action{ Action::None };
    Cost cost{ cost::unknown };
};


using ActionList = std::array<ExtAction, Max_actions>;

struct Stack {
    ActionList* pactions{ nullptr };
    State* pstate{ nullptr };
    Action action{ Action::None };
    Cost cost{ cost::unknown };
    int action_count{ 0 };
    int depth{ 0 };
};

/// Globals
namespace {

    const Params* prm;
    Timer time;

    ActionList sactions[Max_depth + 1];
    std::array<State, Max_depth + 1> states;
    int last_hole = Max_length;

    std::deque<Action> best_actions;

} // namespace

/**
 * Main method to be called from main.
 */
void Agent::solve(const Game& game, int time_limit_ms)
{
    time.reset();
    bool timeout = false;

    init(game, time_limit_ms);

    std::array<Stack, Max_depth> stack;
    stack.fill(Stack{});
    Stack* ss = &stack[0];

    for (int depth=0; depth < Max_depth; ++depth)
    {
        // Initialize the next stack depth for the next iteration
        (ss + depth)->depth = depth;
        (ss + depth)->pstate = &states[depth];
        (ss + depth)->pactions = &sactions[depth];

        Cost best_cost = depth_first_search(depth, timeout, ss);

        // std::stable_sort(pa->begin(), pa->end());

        if (best_cost <= cost::known_win) {
            // TODO: Stop searching and recreate the winning sequence
            break;
        }

        if (timeout) {
            // TODO: output the best action so far, apply it on state
            // and do the next search without incrementing the depth
            // since the root just moved up in depth
            ++ss;
            continue;
        }
    }
}

Action Agent::next_action() const
{
    return sactions[actions_out_count++][0];
}

void Agent::init(const Game& game, int time_limit_ms)
{
    prm = game.parameters();
    last_hole = game.find_last_hole();

    states[0] = *game.state();

    this->time_limit_ms = time_limit_ms;

    std::for_each(&sactions[0], &sactions[Max_depth], [](auto& al) {
        al.fill(ExtAction {});
    });
}

namespace {

    size_t inline road_length()
    {
        return prm->road[0].size();
    }

    int inline n_bikes(const State& s)
    {
        return std::count(s.bikes.begin(), s.bikes.end(), 1);
    }

    /// The greatest lower bound for the number of turns in
    /// which it is possible to reach the end of the road
    inline Cost future_cost_lb(const State& s)
    {
        int p = s.pos, v = s.speed, t = 0;

        while (p < road_length()) {
            v += 1;
            p += v;
            t += 1;
        }

        return t;
    }

    /// Compute a penalty if bikes were lost
    Cost inline action_cost(const State& before, const State& after)
    {
        int bikes_lost = n_bikes(after) - n_bikes(before);
        return 1 + 5 * bikes_lost;
    }

    bool inline at_eor(const State& s)
    {
        return s.pos >= prm->road[0].size();
    }

    bool inline is_past_holes(const State& s)
    {
        return s.pos >= last_hole;
    }

    int inline n_turns_left(const State& s)
    {
        return Max_depth - s.turn;
    }

    bool inline is_lost(const State& s)
    {
        return future_cost_lb(s) > n_turns_left(s)
            || n_bikes(s) < prm->min_bikes;
    }

} // namespace

const std::vector<ExtAction>& Agent::generate_actions(const State& s) const
{
    static std::vector<ExtAction> ret;
    ret.clear();

    auto actions = game.valid_actions(s);

    std::transform(actions.begin(), actions.end(), std::back_inserter(ret), [](auto a) {
        return ExtAction(a);
    });

    return ret;
}

Cost Agent::depth_first_search(int depth, bool& timeout, Stack* ss) const
{
    if (is_lost(*ss->pstate))
        return cost::known_loss;

    if (is_past_holes(*ss->pstate)) {
        ss->pactions->fill(ExtAction(Action::None)); // Use as sentinel
        return cost::known_win;
    }

    if (depth == 0) {
        return future_cost_lb(*ss->pstate);
    }

    ActionList actions {
        ExtAction{ Action::None },
        ExtAction{ Action::None },
        ExtAction{ Action::None },
        ExtAction{ Action::None },
        ExtAction{ Action::None }
    };
    ActionList::iterator local_pa;
    ActionList::iterator local_pa_end;

    bool on_main_line = ss->action_count == 0;

    if (!on_main_line) {
        const auto& candidates = generate_actions(*ss->pstate);
        std::copy(candidates.begin(), candidates.end(), actions.begin());
        ss->pactions = &actions;
    }

    local_pa = ss->pactions->begin();
    local_pa_end = std::find(local_pa, ss->pactions->end(), ExtAction{ Action::None });

    Action best_action = *local_pa;      // Implicit downcast from ExtAction to Action
    Cost best_cost = cost::max;
    
    for (auto it = local_pa; it != local_pa_end; ++it)
    {
        ++ss->action_count;

        /// The actions are sorted so this would mark the end
        if (it->cost >= cost::known_loss)
            return cost::known_loss;

        /// Apply the action
        (ss + 1)->action = *it;
        game.apply(*ss->pstate, *it, *(ss + 1)->pstate);

        /// Recursively evaluate the future cost
        Cost cost = action_cost(*ss->pstate, *(ss + 1)->pstate)
            + depth_first_search(depth - 1, timeout, ss + 1);

        /// Return immediately if winning line found
        if (it->cost == cost::known_win) {
            ss->pactions->front().action = best_action;
            return it->cost;
        }

        /// Update best cost
        if (it->cost < best_cost) {
            best_cost = it->cost;
            best_action = *it;
        }

        /// Mark the paths that are known losses on main lain
        if (on_main_line && it->cost == cost::max)
            it->cost = cost::known_loss;
    }

    /// Keep the actions of main line
    if (on_main_line)
        std::stable_sort(ss->pactions->begin(), ss->pactions->end());

    return best_cost;
}

} // namespace tb
