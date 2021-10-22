#include "agent.h"
#include "dp.h"

#include <algorithm>
#include <array>
#include <string>

namespace {

/// What is internally meant by an action
struct AAction {
    int pos;
    int floor;
};

constexpr int max_actions = dp::max_width;
using Action = AAction;

enum class Cost {
    Zero = 0,
    KnownWin = 1,
    Unknown = 50,
    KnownLoss = 99,
    Infinite = 200,
};

std::underlying_type_t<Cost> to_int(Cost c)
{
    return (std::underlying_type_t<Cost>)c;
}

/// Globals
dp::State states[dp::max_turns + 1];
dp::State* ps = &states[0];
const dp::GameParams* params;

using ActionList = std::array<Action, max_actions>;
std::array<ActionList, dp::max_turns + 1> actions;
ActionList* as = &actions[0];

/// The actions to consider at each floor
std::vector<std::vector<int>> candidates;

/// Used to populate those actions
void init_actions();

} // namespace

void Agent::init(const dp::Game& g)
{
    std::fill(&states[0], &states[dp::max_turns], dp::State {});

    for (int i = 0; i < dp::max_turns + 1; ++i) {
        std::fill(&actions[i][0], &actions[i][max_actions - 1], Action {});
    }

    params = g.get_params();
    *ps++ = *g.state();
    as = &actions[1];

    init_actions();
}

dp::Action Agent::best_choice()
{
    auto action = dp::Action::Wait;

    return action;
}

namespace {
using namespace dp;

bool operator<(const Cost a, const Cost b)
{
    return to_int(a) < to_int(b);
}
Cost operator+(Cost a, Cost b)
{
    return Cost(to_int(a) + to_int(b));
}

/// Lower bound on cost (manhattan distance to exit)
inline Cost cost_lb(const State& s)
{
    return Cost(params->exit_pos - s.pos + params->exit_floor - s.floor);
}

/// Cost to date
inline Cost prev_cost(const State& s)
{
    return Cost(s.turn);
}

/// True if the game is lost
inline bool is_lost(const State& s)
{
    return cost_lb(s) + prev_cost(s) > Cost(params->max_round)
        || s.used_clones > params->max_clones;
}

/// True if the game is won
inline bool is_won(const State& s)
{
    return s.floor == params->exit_floor
        && s.pos == params->exit_pos
        && !is_lost(s);
}

/// Only elevators under or one-off of an elevator
/// above need to be considered
void init_actions()
{
    std::array<std::array<bool, max_width>, max_height> seen;
    for (int i = 0; i < max_height; ++i) {
        seen[i].fill(false);
    }
    const int max_gap = params->n_add_elevators;

    /// Start at the top
    for (auto el = params->elevators.rbegin(); el != params->elevators.rend(); ++el) {
        /// Considers all three columns adjacent to the elevator
        for (int i = el->pos - 1; i < el->pos + 2; ++i) {
            if (i < 0 || i > params->width - 1)
                continue;

            /// Go downwards
            for (int h = el->floor; h > (el->floor - max_gap) && h >= 0; --h) {
                /// Register the positions in reach of the max_gap
                seen[h][i] = true;
            }
        }
    }
    /// Save the result in the global vector
    for (int i = 0; i < params->height; ++i) {
        std::copy(seen[i].begin(), seen[i].begin() + params->width, std::back_inserter(candidates.emplace_back()));
    }
}

/// Method to evaluate the cost of an action on a state
Cost cost(const State& s, const Action a)
{
    const bool block = (s.dir == State::Left) && (a.pos > s.pos)
        || (s.dir == State::Right) && (a.pos < s.pos);

    const bool elevator = a.floor > s.floor;

    const int distance = std::abs(a.pos - s.pos);

    return Cost(block * 3 + elevator * 3 + distance + 1);
}

inline State::Dir opposite(State::Dir d)
{
    return d == State::Left ? State::Right : State::Left;
}

/// The internal method we use to simulate games.
State& apply(State& s, const AAction a)
{
    const bool block = (s.dir == State::Left) && (a.pos > s.pos)
        || (s.dir == State::Right) && (a.pos < s.pos);

    const bool elevator = a.floor > s.floor;

    const int distance = std::abs(a.pos - s.pos);

    State& nex = *ps++;
    nex = s;
    nex.prev = &s;

    nex.pos = a.pos;
    nex.floor = a.floor;
    nex.turn -= block * 3 + elevator * 3 + distance + 1;
    nex.used_clones -= (block || elevator);
    nex.used_elevators -= elevator;
    nex.dir = block ? opposite(nex.dir) : nex.dir;

    return nex;
}

} // namespace
