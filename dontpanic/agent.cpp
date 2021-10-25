#include "agent.h"
#include "dp.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <iostream>
#include <string>

namespace {

/// Internally, an action is encoded by the position
/// at which the elevator will be taken on a state's floor
using AAction = int;

/// Same for elevators, since we split them into floors
using AElevator = int;

    constexpr int max_actions = dp::max_width;

using Cost = int;

/// Globals
dp::State states[dp::max_turns + 1];
dp::State* ps = &states[0];
const dp::GameParams* params;

using ActionList = std::vector<AAction>;

/// The elevators, split into floors
std::vector<std::vector<AElevator>> elevators;

/// The actions to consider at each floor
std::vector<ActionList> candidates;

/// A buffer for populating the search tree
ActionList actions_buffer;

/// Used to populate the elevators
void init_elevators();

/// Used to populate the candidates
void init_actions();

/// A queue for real Actions to be played
std::deque<dp::Action> best_actions;

/// The dfs method to implement the incremental
/// dfs Agent::search() method
Cost depth_first_search(const dp::State& s, int depth, bool& found);

} // namespace


void Agent::init(const dp::Game& g)
{
    std::fill(&states[0], &states[dp::max_turns], dp::State {});

    params = g.get_params();
    states[0] = *g.state();
    ps = &states[1];

    init_elevators();
    init_actions();
}

dp::Action Agent::best_choice()
{
    if (best_actions.empty())
        return dp::Action::None;

    dp::Action action = best_actions.front();
    best_actions.pop_front();
    return action;
}

void Agent::search()
{
    assert(ps == &states[1]);

    bool found = false;

    for (int depth = 0; depth < params->max_round; ++depth) {
        depth_first_search(states[0], depth, found);
        if (found)
            break;
    }
}

namespace {
using namespace dp;

/// True if the game is lost
inline bool is_lost(const State& s)
{
    return s.clones < 0 || s.turn < 0 || s.player_elevators < 0 || s.floor > params->exit_floor;
}

/// True if the game is won
inline bool is_won(const State& s)
{
    const bool at_exit = s.floor == params->exit_floor;
    const bool lost = is_lost(s);
    if (at_exit && lost) {
        std::cerr << "WARNING: state at exit but is_lost returns true"
            << std::endl;
    }
    return at_exit && !lost;
}

inline bool on_elevator(int pos, int floor) {
    return std::find_if(params->elevators.begin(), params->elevators.end(), [p=pos, f=floor](const auto& el){
            return el.floor == f && el.pos == p;
        }) == params->elevators.end();
}

inline bool is_player_elevator(const State& s, const AAction a)
{
    return !on_elevator(a, s.floor);
}

inline bool is_block(const State& s, const AAction a)
{
    return (s.dir == Dir::Right) && (a < s.pos)
        || (s.dir == Dir::Left) && (a > s.pos);
}

inline Dir opposite(Dir d)
{
    return d == Dir::Left ? Dir::Right : Dir::Left;
}

inline Cost past_cost(const State& s)
{
    return params->max_round - s.turn;
}

inline int distance(int fa, int pa, int fb, int pb)
{
    return std::abs(fb - fa) + std::abs(pb - pa);
}

/// Number of turns taken to apply action a on state s.
inline Cost action_cost(const State& s, const AAction a)
{
    const int dist = distance(s.floor, s.pos, s.floor+1, a);

    return 3 + 3 * is_block(s, a) + dist;  // The first 3 is for the elevator action
}

inline Cost future_cost_lb(const State& s)
{
    return distance(s.floor, s.pos, params->exit_floor, params->exit_pos);
}

/// Split the elevators into floors and order them wrt their position
void init_elevators()
{
    for (int i=0; i<params->exit_floor + 1; ++i) {
        auto& elevs = elevators.emplace_back();
        for (const auto& el : params->elevators) {
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
    for (int i = 0; i < params->height; ++i) {
        seen.push_back(std::vector<bool>(params->width, false));
    }
    const int max_gap = params->n_add_elevators;

    /// add the exit itself as a candidate target
    seen[params->exit_floor][params->exit_pos] = true;

    /// add candidates for elevators under the exit
    for (int h = params->exit_floor - 1; h >= (params->exit_floor - max_gap) && h >= 0; --h) {
        seen[h][params->exit_pos] = true;
    }

    /// Start at the top of the elevators list
    for (int fl = params->exit_floor; fl >= 0; --fl) {
        /// Considers all three columns adjacent to the elevator
        for (AElevator el : elevators[fl]) {
            for (int p = el - 1; p < el + 2; ++p) {
                if (p < 0 || p > params->width - 1)
                    continue;
                /// Go downwards
                for (int f = fl; f > (fl - max_gap) && f >= 0; --f) {
                    /// Register the positions in reach of the max_gap
                    seen[f][p] = true;
                }
            }
        }
    }
    /// Save the result in the global vector
    for (int i = 0; i < params->exit_floor; ++i)
        std::transform(seen.begin(), seen.end(), std::back_inserter(candidates), [](const auto& row){
            ActionList ret;
            for (int p=0; p<row.size(); ++p)
                if (row[p])
                    ret.push_back(p);
            return ret;
        });
}

/// The internal method we use to simulate games.
State& apply(const State& s, const AAction a)
{
    State& nex = *ps++;
    nex.dir = is_block(s, a) ? opposite(nex.dir) : nex.dir;
    nex.pos = a;
    nex.floor = s.floor == params->exit_floor ? nex.floor : s.floor + 1;
    nex.turn = s.turn - action_cost(s, a);
    nex.clones = s.clones - 1 - is_block(s, a);
    nex.player_elevators = s.player_elevators - 1;

    nex.prev = const_cast<State*>(&s);
    return nex;
}

/// Transform an AAction into a sequence of actual dp::Actions once a solution is found
void populate_actions(const State& s, const AAction a, std::deque<dp::Action>& q)
{
    if (s.floor < params->exit_floor) {
        for (int i=0; i<3; ++i)
            q.push_front(dp::Action::Wait);
        q.push_front(dp::Action::Elevator);
    }

    for (int i=0; i < distance(s.floor, s.pos, s.floor, a); ++i)
        q.push_front(dp::Action::Wait);

    if (is_block(s, a))
    {
        for (int i=0; i<3; ++i)
            q.push_front(dp::Action::Wait);
        q.push_front(dp::Action::Block);
    }
}

/// Draw from the candidates to get the reachable actions
const std::vector<AAction>& get_valid_actions(const State& s)
{
    actions_buffer.clear();

    auto el_right = std::find_if(elevators[s.floor].begin(), elevators[s.floor].end(), [p=s.pos](AElevator el){
        return el >= p;
    });

    int first_el_right = params->width - 1;
    if (el_right != elevators[s.floor].end()) {
        first_el_right = *el_right;
    }

    auto el_left = std::find_if(elevators[s.floor].rbegin(), elevators[s.floor].rend(), [p=s.pos](AElevator el){
        return el < p;
    });

    int first_el_left = 0;
    if (el_left != elevators[s.floor].rend()) {
        first_el_left = *el_left;
    }

    const auto& cands = candidates[s.floor];
    for (AAction a : candidates[s.floor]) {
        if (a < first_el_left || a > first_el_right)
            continue;
        actions_buffer.push_back(a);
        if (a >= first_el_right)
            break;
    }

    return actions_buffer;
}

Cost depth_first_search(const State& s, int depth, bool& found)
{
    if (depth == 0) {
        Cost estimate = future_cost_lb(s);
        found = estimate == 0;
        return past_cost(s) + estimate;
    }

    std::array<AAction, dp::max_width> actions;
    actions.fill(-1);
    const auto& valid_actions = get_valid_actions(s);

    std::copy(valid_actions.begin(), valid_actions.end(), actions.begin());

    // Sort by increasing distance to the exit
    std::sort(actions.begin(), actions.begin() + valid_actions.size(), [f=s.floor, ef=params->exit_floor, ep=params->exit_pos](AAction a, AAction b){
        return distance(a, f, ep, ef) < distance(b, f, ep, ef);
    });

    AAction best_action = -1;
    Cost best_cost = 1000;

    for (auto a = actions.begin(); a != actions.begin() + valid_actions.size(); ++a) {
        State& st = apply(s, *a);
        Cost cost = depth_first_search(st, depth - 1, found);
        --ps;

        if (found) {
            populate_actions(*(ps - 1), *a, best_actions);
            return cost;
        }

        if (cost < best_cost) {
            best_cost = cost;
            best_action = *a;
        }
    }

    return best_cost;
}

} // namespace
