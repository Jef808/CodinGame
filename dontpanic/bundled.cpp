#define RUNNING_OFFLINE 1
#define EXTRACTING_ONLINE_DATA 0

#undef _GLIBCXX_DEBUG // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

#pragma GCC target("bmi,bmi2,lzcnt,popcnt") // bit manipulation
#pragma GCC target("movbe") // byte swap
#pragma GCC target("aes,pclmul,rdrnd") // encryption
#pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include <vector>

namespace dp {

constexpr int max_turns = 200;
constexpr int max_height = 15;
constexpr int max_width = 100;
constexpr int max_n_clones = 50;
constexpr int max_n_elevators = 100;
constexpr int max_time_ms = 100;

enum class Type { Elevator,
    Clone };
enum class Dir { Right,
    Left };
struct Entity {
    Type type;
    int pos;
    int floor;
    std::optional<Dir> dir;
};

struct GameParams {
    int height;
    int width;
    int max_round;
    int exit_floor;
    int exit_pos;
    int max_clones;
    int n_add_elevators;
    int entry_pos;
    std::vector<Entity> elevators;
};

struct State {
    Dir dir;
    int floor;
    int pos;
    int turn;
    int clones;
    int player_elevators;
    State* prev;
};

enum class Action {
    None,
    Wait,
    Block,
    Elevator
};

class Game {
public:
    Game() = default;
    void init(std::istream&);
    const State* state() const;
    const GameParams* get_params() const;
    const State& get_root_state() const;

private:
    State* ps;
};

inline const State* Game::state() const { return ps; }

} // namespace dp

extern void extract_online_init();

namespace dp {

/// Globals
namespace {

    GameParams params {};
    State root_state {};

} // namespace

void Game::init(std::istream& _in)
{
    int n_elevators;
    _in >> params.height
        >> params.width
        >> params.max_round
        >> params.exit_floor
        >> params.exit_pos
        >> params.max_clones
        >> params.n_add_elevators
        >> n_elevators;

    for (int i = 0; i < n_elevators; ++i) {
        auto& el = params.elevators.emplace_back();
        _in >> el.floor >> el.pos;
    }

    ps = &root_state;

    // First turn because not everything is initialized
    char d;
    _in >> ps->floor >> ps->pos >> d;
    _in.ignore();

    ps->dir = (d == 'L' ? Dir::Left : Dir::Right);
    ps->clones = params.max_clones;
    ps->player_elevators = params.n_add_elevators;
    ps->turn = params.max_round;

    params.entry_pos = ps->pos;

    // Order the elevators by floor then by pos
    std::sort(params.elevators.begin(), params.elevators.end(), [](const auto& a, const auto& b) {
        return a.floor < b.floor
            || (a.floor == b.floor && a.pos < b.pos);
    });
}

const GameParams* Game::get_params() const
{
    return &params;
}

#if EXTRACTING_ONLINE_DATA
#include <sstream>

void extract_online_init()
{
    Game game;
    game.init(std::cin);

    std::stringstream ss {};

    ss << params.height << ' '
       << params.width << ' '
       << params.max_round << ' '
       << params.exit_floor << ' '
       << params.exit_pos << ' '
       << params.max_clones << ' '
       << params.n_add_elevators << ' '
       << params.elevators.size()
       << '\n';

    for (const auto& el : params.elevators) {
        ss << el.floor << ' '
           << el.pos
           << '\n';
    }

    char d;
    ss << ps->floor << ' '
       << ps->pos << ' '
       << d;
    << (d == 'L' ? Dir::Left : Dir::Right)
    << '\n';

    out << ss.str() << std::endl;
    out << "Successfully read all data." << std::endl;
}

#endif // EXTRACTING_ONLINE_DATA

} // namespace dp

#ifndef AGENT_H_
#define AGENT_H_

namespace dp {
class Game;
enum class Action;

namespace agent {
    void init(const dp::Game&);

    /// The main search method
    void search();

    /// The best choice to date
    dp::Action best_choice();

} // namespace agent
} // namespace dp

#endif // AGENT_H_

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

/// A queue for real Actions to be played
std::deque<dp::Action> best_actions;

bool depth_first_search(const dp::State& s, int max_depth);

} // namespace

namespace dp::agent {

void init(const dp::Game& g)
{
    std::fill(&states[0], &states[dp::max_turns], dp::State {});

    gparams = g.get_params();
    states[0] = *g.state();
    ps = &states[1];

    init_elevators();
    init_actions();
    cost_buffer.reserve(params.width);
    std::fill_n(std::back_inserter(cost_buffer), params.width, cost_max);
}

dp::Action best_choice()
{
    if (best_actions.empty())
        return dp::Action::None;

    dp::Action action = best_actions.front();
    best_actions.pop_front();
    return action;
}

void search()
{
    assert(ps == &states[1]);

    bool found = depth_first_search(states[0], params.exit_floor);

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
    for (int i = 0; i <= params.exit_floor; ++i) {
        auto& elevs = elevators.emplace_back();
        for (const auto& el : params.elevators) {
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
    for (int i = 0; i <= params.exit_floor; ++i) {
        seen.push_back(std::vector<bool>(params.width, false));
    }

    n_empty_floors_above.push_back(std::count_if(elevators.begin(), elevators.begin() + params.exit_floor, [](const auto& els) {
        return els.empty();
    }));
    for (int i = 0; i < params.exit_floor - 1; ++i) {
        n_empty_floors_above.push_back(n_empty_floors_above[i] - elevators[i].empty());
    }
    n_empty_floors_above.push_back(0);
    n_empty_floors_above.push_back(0);

    const int max_gap = params.n_add_elevators;
    auto elevator_gap = [max_gap](int floor) {
        return max_gap - n_empty_floors_above[floor];
    };

    /// add a target at the exit itself
    seen[params.exit_floor][params.exit_pos] = true;

    /// add targets under the exit
    for (int f = params.exit_floor, p = params.exit_pos; f >= params.exit_floor - max_gap && f >= 0; --f) {
        seen[f][p] = true;
    }

    /// add targets at the columns before any elevators blocking the exit
    {
        auto first_rightof_exit = std::find_if(elevators[params.exit_floor].begin(), elevators[params.exit_floor].end(), [p = params.exit_pos](const auto& el) {
            return el > p;
        });
        auto first_leftof_exit = std::find_if(elevators[params.exit_floor].rbegin(), elevators[params.exit_floor].rend(), [p = params.exit_pos](const auto& el) {
            return el < p;
        });

        if (first_rightof_exit != elevators[params.exit_floor].end()) {
            for (int f = params.exit_floor - 1, p = *first_rightof_exit - 1; f >= params.exit_floor - 1 - max_gap && f >= 0; --f) {
                seen[f][p] = true;
            }
        }
        if (first_leftof_exit != elevators[params.exit_floor].rend()) {
            for (int f = params.exit_floor - 1, p = *first_leftof_exit + 1; f >= params.exit_floor - 1 - max_gap && f >= 0; --f) {
                seen[f][p] = true;
            }
        }
    }

    /// add targets around elevator columns
    for (int fl = params.exit_floor - 1; fl >= 0; --fl) {
        for (AElevator el : elevators[fl]) {
            /// Considers all three columns adjacent to the elevator
            for (int p = el - 1; p < el + 2; ++p) {
                /// Go downwards by the `allowed_gap` amount of floors, if column isn't bad
                if (p < 0 || p > params.width - 1)
                    continue;
                for (int f = fl; f >= (fl - elevator_gap(fl)) && f >= 0; --f) {
                    seen[f][p] = true;
                }
            }
        }
    }
    /// Save the result in the global vector
    for (int i = 0; i <= params.exit_floor; ++i)
        std::transform(seen.begin(), seen.end(), std::back_inserter(candidates), [](const auto& target) {
            ActionList ret;
            for (int p = 0; p < params.width; ++p)
                if (target[p])
                    ret.push_back(p);
            return ret;
        });
}

inline bool at_exit_floor(const State& s)
{
    return s.floor == params.exit_floor;
}

inline bool at_exit(const State& s)
{
    return at_exit_floor(s) && s.pos == params.exit_pos;
}

inline bool on_elevator(int pos, int floor)
{
    return std::find_if(params.elevators.begin(), params.elevators.end(), [p = pos, f = floor](const auto& el) {
        return el.floor == f && el.pos == p;
    }) != params.elevators.end();
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
    const bool is_exit_floor = s.floor == params.exit_floor;
    const int dist = distance(s.pos, s.floor, a, s.floor) + (1 - is_exit_floor);
    return 3 * is_block(s, a) + dist + 3 * is_player_elevator(s, a);
}

inline Cost future_cost_lb(const State& s)
{
    return distance(s.floor, s.pos, params.exit_floor, params.exit_pos)
        + (1 - (at_exit_floor(s)));
}

inline Cost future_cost_lb(const State& s, const AAction a)
{
    return action_cost(s, a)
        + distance(s.floor + (1 - at_exit_floor(s)), a, params.exit_floor, params.exit_pos);
}

/// True if the game is lost
inline bool is_lost(const State& s)
{
    return future_cost_lb(s) > s.turn
        || n_empty_floors_above[s.floor] > s.player_elevators
        || s.clones < 0;
    //|| s.floor > params.exit_floor
    //|| s.pos < 0 || s.pos > params.width - 1;
}

State& apply(const State& s, const AAction a)
{
    const bool is_b = is_block(s, a);
    const bool is_ef = s.floor == params.exit_floor;
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
    if (s.floor < params.exit_floor)
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

    int first_el_right = params.width - 1;
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

#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#if RUNNING_OFFLINE
#include <fmt/format.h>
#endif

std::ostream& operator<<(std::ostream& _out, const dp::Action a)
{
    switch (a) {
    case dp::Action::Wait:
        return _out << "WAIT";
    case dp::Action::Block:
        return _out << "BLOCK";
    case dp::Action::Elevator:
        return _out << "ELEVATOR";
    default:
        return throw "Action::None", _out << "WARNING: Chose Action::None";
    }
}

void ignore_turn(std::istream& _in)
{
    static std::string buf;
    buf.clear();
    std::getline(_in, buf);
    _in.ignore();
}

int main(int argc, char* argv[])
{
#if RUNNING_OFFLINE
    if (argc < 2) {
        fmt::print(stderr, "USAGE: {} [Test number]\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string fn;
    fmt::format_to(std::back_inserter(fn), "../data/test{}.txt", argv[1]);

    std::ifstream ifs { fn.data() };

    if (!ifs) {
        fmt::print("Failed to open input file {}\n", fn);
        return EXIT_FAILURE;
    }

    Game game;
    game.init(ifs);

#else
#if EXTRACTING_ONLINE_DATA

    extract_online_init();
    return EXIT_SUCCESS;

#endif

    Game game;
    game.init(std::cin);

#endif

    agent::init(game);

    auto start = std::chrono::steady_clock::now();

    agent::search();

    auto time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
    std::cerr << std::setprecision(4)
              << "Time taken: "
              << time / 1000 << "ms" << std::endl;

    Action action = agent::best_choice();
    while (action != Action::None) {
        std::cout << action
                  << std::endl;
        action = agent::best_choice();

#if RUNNING_OFFLINE

        continue;
#endif

        ignore_turn(std::cin);
    }

    return EXIT_SUCCESS;
}
