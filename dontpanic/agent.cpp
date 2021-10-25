#include "agent.h"
#include "dp.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <iostream>
#include <numeric>
#include <string>

namespace {

/// Internally, an action is encoded by the position
/// at which the elevator will be taken on a state's floor
using AAction = int;

/// Same for elevators, since we split them into floors
using AElevator = int;

constexpr int max_actions = dp::max_width;

using Cost = int;
constexpr int cost_max = dp::max_width * dp::max_height + 1;

/// Globals
dp::State states[dp::max_turns + 1];
dp::State* ps = &states[0];
const dp::GameParams* gparams;

using ActionList = std::vector<AAction>;

/// The elevators, split into floors
std::vector<std::vector<AElevator>> elevators;
std::vector<int> n_empty_floors_above;

/// The actions to consider at each floor
std::vector<ActionList> candidates;

/// A buffer for populating the search tree
ActionList actions_buffer;

/// Another buffer to store cost computations
std::vector<int> cost_buffer;

/// Used to populate the elevators
void init_elevators();

/// Used to populate the candidates
void init_actions();

/// Initialize the search
void init(const dp::Game& g);

/// A queue for real Actions to be played
std::deque<dp::Action> best_actions;

bool depth_first_search(const dp::State& s, int max_depth);

} // namespace

namespace dp::agent {

dp::Action best_choice()
{
    if (best_actions.empty())
        return dp::Action::None;

    dp::Action action = best_actions.front();
    best_actions.pop_front();
    return action;
}

void search(const Game& g)
{
    assert(ps == &states[1]);

    init(g);

    bool found = depth_first_search(states[0], gparams->exit_floor);

    assert(found);

    // Have to wait once at exit
    best_actions.push_back(dp::Action::Wait);
}

} // namespace dp::agent

namespace {
using namespace dp;

/// Split the elevators into floors and order them wrt their position
void init_elevators()
{
    for (int i = 0; i <= gparams->exit_floor; ++i) {
        auto& elevs = elevators.emplace_back();
        for (const auto& el : gparams->elevators) {
            if (el.floor == i)
                elevs.push_back(el.pos);
        }
        std::sort(elevs.begin(), elevs.end());
    }
}

/// Only elevators under or one-off of an elevator
/// above need to be considered
void init_actions()
{
    std::vector<std::vector<bool>> seen;
    for (int i = 0; i <= gparams->exit_floor; ++i) {
        seen.push_back(std::vector<bool>(gparams->width, false));
    }

    n_empty_floors_above.push_back(std::count_if(elevators.begin(), elevators.begin() + gparams->exit_floor, [](const auto& els) {
        return els.empty();
    }));
    for (int i = 0; i < gparams->exit_floor - 1; ++i) {
        n_empty_floors_above.push_back(n_empty_floors_above[i] - elevators[i].empty());
    }
    n_empty_floors_above.push_back(0);
    n_empty_floors_above.push_back(0);

    const int max_gap = gparams->n_add_elevators;
    auto elevator_gap = [max_gap](int floor) {
        return max_gap - n_empty_floors_above[floor];
    };

    /// add a target at the exit itself
    seen[gparams->exit_floor][gparams->exit_pos] = true;

    /// add targets under the exit
    for (int f = gparams->exit_floor, p = gparams->exit_pos; f >= gparams->exit_floor - max_gap && f >= 0; --f) {
        seen[f][p] = true;
    }

    /// add targets at the columns before any elevators blocking the exit
    {
        auto first_rightof_exit = std::find_if(elevators[gparams->exit_floor].begin(), elevators[gparams->exit_floor].end(), [p = gparams->exit_pos](const auto& el) {
            return el > p;
        });
        auto first_leftof_exit = std::find_if(elevators[gparams->exit_floor].rbegin(), elevators[gparams->exit_floor].rend(), [p = gparams->exit_pos](const auto& el) {
            return el < p;
        });

        if (first_rightof_exit != elevators[gparams->exit_floor].end()) {
            for (int f = gparams->exit_floor - 1, p = *first_rightof_exit - 1; f >= gparams->exit_floor - 1 - max_gap && f >= 0; --f) {
                seen[f][p] = true;
            }
        }
        if (first_leftof_exit != elevators[gparams->exit_floor].rend()) {
            for (int f = gparams->exit_floor - 1, p = *first_leftof_exit + 1; f >= gparams->exit_floor - 1 - max_gap && f >= 0; --f) {
                seen[f][p] = true;
            }
        }
    }

    /// add targets around elevator columns
    for (int fl = gparams->exit_floor - 1; fl >= 0; --fl) {
        for (AElevator el : elevators[fl]) {
            /// Considers all three columns adjacent to the elevator
            for (int p = el - 1; p < el + 2; ++p) {
                /// Go downwards by the `allowed_gap` amount of floors, if column isn't bad
                if (p < 0 || p > gparams->width - 1)
                    continue;
                for (int f = fl; f >= (fl - elevator_gap(fl)) && f >= 0; --f) {
                    seen[f][p] = true;
                }
            }
        }
    }
    /// Save the result in the global vector
    for (int i = 0; i <= gparams->exit_floor; ++i)
        std::transform(seen.begin(), seen.end(), std::back_inserter(candidates), [](const auto& target) {
            ActionList ret;
            for (int p = 0; p < gparams->width; ++p)
                if (target[p])
                    ret.push_back(p);
            return ret;
        });
}

void init(const dp::Game& g)
{
    std::fill(&states[0], &states[dp::max_turns], dp::State {});

    gparams = g.get_params();
    states[0] = *g.state();
    ps = &states[1];

    init_elevators();
    init_actions();
    cost_buffer.reserve(gparams->width);
    std::fill_n(std::back_inserter(cost_buffer), gparams->width, cost_max);
}

inline bool at_exit_floor(const State& s)
{
    return s.floor == gparams->exit_floor;
}

inline bool at_exit(const State& s)
{
    return at_exit_floor(s) && s.pos == gparams->exit_pos;
}

inline bool on_elevator(int pos, int floor)
{
    return std::find_if(gparams->elevators.begin(), gparams->elevators.end(), [p = pos, f = floor](const auto& el) {
        return el.floor == f && el.pos == p;
    }) != gparams->elevators.end();
}

/// true if the action sequence ends with the player creating an elevator
inline bool is_player_elevator(const State& s, const AAction a)
{
    return !on_elevator(a, s.floor) && !at_exit_floor(s);
}

/// True if the action sequence starts with a block
inline bool is_block(const State& s, const AAction a)
{
    return ((s.dir == Dir::Right) && (a < s.pos))
        || ((s.dir == Dir::Left) && (a > s.pos));
}

inline Dir opposite(Dir d)
{
    return d == Dir::Left ? Dir::Right : Dir::Left;
}

inline int distance(int fa, int pa, int fb, int pb)
{
    return std::abs(fb - fa) + std::abs(pb - pa);
}

/// Number of turns taken to apply action a on state s.
inline Cost action_cost(const State& s, const AAction a)
{
    const bool is_exit_floor = s.floor == gparams->exit_floor;
    const int dist = distance(s.pos, s.floor, a, s.floor) + (1 - is_exit_floor);
    return 3 * is_block(s, a) + dist + 3 * is_player_elevator(s, a);
}

inline Cost future_cost_lb(const State& s)
{
    return distance(s.floor, s.pos, gparams->exit_floor, gparams->exit_pos)
        + (1 - (at_exit_floor(s)));
}

inline Cost future_cost_lb(const State& s, const AAction a)
{
    return action_cost(s, a)
        + distance(s.floor + (1 - at_exit_floor(s)), a, gparams->exit_floor, gparams->exit_pos);
}

/// True if the game is lost
inline bool is_lost(const State& s)
{
    return future_cost_lb(s) > s.turn
        || n_empty_floors_above[s.floor] > s.player_elevators
        || s.clones < 0;
    //|| s.floor > gparams->exit_floor
    //|| s.pos < 0 || s.pos > gparams->width - 1;
}

State& apply(const State& s, const AAction a)
{
    const bool is_b = is_block(s, a);
    const bool is_ef = s.floor == gparams->exit_floor;
    const bool is_pe = is_player_elevator(s, a);

    State& nex = *ps++;
    nex.dir = is_b ? opposite(s.dir) : s.dir;
    nex.pos = a;
    nex.floor = s.floor + (1 - is_ef);
    nex.turn = s.turn
        - 3 * is_b
        - distance(s.floor, s.pos, s.floor, a)
        - 3 * is_pe
        - (1 - is_ef);
    nex.clones = s.clones - is_pe - is_b;
    nex.player_elevators = s.player_elevators - is_pe;

    nex.prev = const_cast<State*>(&s);
    return nex;
}

/// Transform an AAction into a sequence of actual dp::Actions once a solution is found
void populate_actions(const State& s, const AAction a, std::deque<dp::Action>& q)
{
    if (s.floor < gparams->exit_floor)
        q.push_front(dp::Action::Wait);

    if (is_player_elevator(s, a)) {
        for (int i = 0; i < 2; ++i)
            q.push_front(dp::Action::Wait);
        q.push_front(dp::Action::Elevator);
    }

    for (int i = 0; i < distance(s.floor, s.pos, s.floor, a); ++i)
        q.push_front(dp::Action::Wait);

    if (is_block(s, a)) {
        for (int i = 0; i < 2; ++i)
            q.push_front(dp::Action::Wait);
        q.push_front(dp::Action::Block);
    }
}

/// Draw from the candidates to get the reachable actions
const std::vector<AAction>& get_valid_actions(const State& s)
{
    actions_buffer.clear();

    int first_el_right = gparams->width - 1;
    int first_el_left = 0;

    bool ignore_right = false, ignore_left = false;

    if (on_elevator(s.pos, s.floor)) {
        if (s.dir == Dir::Right) {
            first_el_right = s.pos;
            ignore_right = true;
        } else if (s.dir == Dir::Left) {
            first_el_left = s.pos;
            ignore_left = true;
        }
    }

    if (!ignore_right) {
        auto el_right = std::find_if(elevators[s.floor].begin(), elevators[s.floor].end(), [p = s.pos](AElevator el) {
            return el > p;
        });

        if (el_right != elevators[s.floor].end()) {
            first_el_right = *el_right;
        }
    }

    if (!ignore_left) {
        auto el_left = std::find_if(elevators[s.floor].rbegin(), elevators[s.floor].rend(), [p = s.pos](AElevator el) {
            return el < p;
        });

        if (el_left != elevators[s.floor].rend()) {
            first_el_left = *el_left;
        }
    }

    const auto& cands = candidates[s.floor];
    std::fill(cost_buffer.begin(), cost_buffer.end(), cost_max);

    for (AAction a : candidates[s.floor]) {
        if (a >= first_el_left
            && a <= first_el_right) {
            /// Skip action if not enough turns left
            cost_buffer[a] = future_cost_lb(s, a);
            if (cost_buffer[a] > s.turn)
                continue;
            actions_buffer.push_back(a);
        }
    }

    std::sort(actions_buffer.begin(), actions_buffer.end(), [](AAction a, AAction b) {
        return cost_buffer[a] < cost_buffer[b];
    });

    return actions_buffer;
}

bool depth_first_search(const State& s, int max_depth)
{
    if (is_lost(s)) {
        return false;
    }
    if (at_exit(s)) {
        return true;
    }

    std::array<AAction, dp::max_width> actions;
    actions.fill(-1);
    const auto& valid_actions = get_valid_actions(s);
    const int n_actions = valid_actions.size();

    std::copy(valid_actions.begin(), valid_actions.end(), actions.begin());

    for (auto a = actions.begin(); a != actions.begin() + n_actions; ++a) {
        State& st = apply(s, *a);
        bool found = depth_first_search(st, max_depth - 1);
        --ps;

        if (found) {
            populate_actions(s, *a, best_actions);
            return true;
        }
    }

    return false;
}

} // namespace
