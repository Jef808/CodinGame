#include "agent.h"
#include "tb.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>


namespace tb {

/// Globals
namespace {

    const Params* prm;

    std::array<State, Max_depth + 1> states;
    State* ps = &states[0];
    int last_hole = Max_length;

    std::deque<Action> best_actions;

} // namespace

void Agent::init(const Game& game)
{
    prm = game.parameters();
    last_hole = game.find_last_hole();

    states[0] = *game.state();
    ps = &states[0];
}

Action Agent::best_action() const
{
    if (best_actions.empty()) {
        return Action::Speed;
    }

    Action action = best_actions.front();
    best_actions.pop_front();

    game.apply(*ps, action, *(ps+1));

    ++ps;

    return action;
}

/**
 * Main method to be called from main.
 */
void Agent::search(const Game& game)
{
    int max_depth = Max_depth;

    bool found = depth_first_search(*ps++, max_depth);
    --ps;

    std::cerr << "Agent::search: "
        << (found ? "found " : "did not find a ")
        << "solution" << std::endl;
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
int inline future_cost_lb(const State& s)
{
    int p = s.pos, v = s.speed, t = 0;

    while (p < road_length()) {
        v += 1;
        p += v;
        t += 1;
    }

    return t;
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

bool Agent::depth_first_search(const State& s, int max_depth) const
{
    if (max_depth == 0) {
        std::cerr << "\nReached max depth"
            << std::endl;
    }

    if (is_lost(s)) {
        return false;
    }

    if (is_past_holes(s)) {
        std::cerr << "\nAgent::dfs: state is past holes!"
            << std::endl;

        return true;
    }

    Action actions[5] {
        Action::None,
        Action::None,
        Action::None,
        Action::None,
        Action::None
    };

    const auto& valid_actions = game.valid_actions(s);
    std::copy(valid_actions.begin(), valid_actions.end(), &actions[0]);
    bool found = false;

    for (auto& a : actions) {
        if (a == Action::None)
            break;

        game.apply(s, a, *ps);
        found = depth_first_search(*ps++, max_depth - 1);
        --ps;

        if (found) {
            best_actions.push_front(a);
            break;
        }
    }

    return found;
}

} // namespace tb
