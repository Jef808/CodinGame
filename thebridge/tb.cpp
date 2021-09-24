#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <iostream>
#include <fstream>
#include <limits>
#include <optional>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

constexpr size_t Max_depth = 50;

/**
 * The mutable part of the game.
 */
struct State {
    size_t pos;
    size_t speed;
    std::array<bool, 4> bikes;
    int turn;
    State* prev;
};

/**
 * The actions acting on States.
 */
enum class Action {
    None=0, Speed=1, Jump=2, Up=3, Down=4, Slow=5, Wait=6, Null=7
};
std::ostream& operator<<(std::ostream& out, const Action a);
size_t to_int(Action a) { return static_cast<int>(a); }

/**
 * The immutable part of the game.
 */
struct GameParams {
    enum class Cell { Bridge, Hole };
    typedef std::array<std::vector<Cell>, 4> Road;
    Road road;
    int start_bikes;
    int min_bikes;
};

GameParams params;

/**
 * Class implementing the game simulation.
 */
class Game {
public:
    enum class Status { Loss=0, Unknown=1, Win };
    using ActionList = std::vector<Action>;

    static void init(std::istream&);
    Game() = default;
    void set(State& st);
    void apply(const Action a, State& st);
    void undo();
    bool is_won() const;
    bool is_lost() const;
    int turn() const { return pstate->turn; }
    int pos() const { return pstate->pos; }
    std::pair<bool, const ActionList&> candidates() const;

private:
    State* pstate;
    static GameParams* pparams;
};

void Game::init(std::istream& _in) {
    std::string buf;

    _in >> params.start_bikes
        >> params.min_bikes; _in.ignore();

    for (int i=0; i<4; ++i)
    {
        std::getline(_in, buf);
        std::transform(buf.begin(), buf.end(), std::back_inserter(params.road[i]),
                       [](const auto c){ return c == '0' ? GameParams::Cell::Hole : GameParams::Cell::Bridge; });
    }

    Game::pparams = &params;
}

State input_turn(std::istream& _in) {
    State state;
    std::stringstream ss{};
    int x, y, a;  // x-coord, y-coord, active-or-not
    _in >> state.speed; _in.ignore();

    for (int i=0; i<4; ++i)
    {
        _in >> x >> y >> a; _in.ignore();

        ss << x << ' ' << y << ' ' << a << std::endl;
        state.bikes[i] = (a == '1');
    }
    state.pos = x;
    return state;
}

void input_turn_ignore(std::istream& _in) {
    std::string buf;
    for (int i=0; i<params.start_bikes; ++i) {
        std::getline(_in, buf);
    }
}

inline void Game::set(State& st) {
    pstate = &st;
}

inline int n_bikes(const State& s) {
    return std::count(s.bikes.begin(), s.bikes.end(), true);
}

inline bool Game::is_lost() const {
    return pstate->turn > 50 || n_bikes(*pstate) < pparams->min_bikes;
}

inline bool Game::is_won() const {
    return pstate->pos >= pparams->road[0].size();
}

std::array<int, 4> next_holes(const State& s, const GameParams::Road& r) {
    using Cell = GameParams::Cell;
    std::array<int, 4> ret;
    for (size_t i=0; i<4; ++i) {
        auto it = std::find(r[i].begin() + s.pos + 1, r[i].end(), Cell::Hole);
        ret[i] = (it == r[i].end()
                    ? std::numeric_limits<int>::max()
                    : std::distance(r[i].begin(), it));
    }
    return ret;
}

// List of actions such that the number of bikes wouldn't
// drop below the minimum amount required.
// TODO: Give the list of previous actions as parameter, so further
// filter the result. e.g. if there was a Slow after the last jump,
// up or down, we shouldn't consider Speed and vice versa
std::pair<bool, const Game::ActionList&> Game::candidates() const {
    using Cell = GameParams::Cell;
    static std::vector<Action> cands;
    cands.clear();

    std::array<int, 4> nexhole = next_holes(*pstate, pparams->road);
    for (size_t i=0; i<4; ++i) {
        auto it = std::find(pparams->road[i].begin() + pstate->pos + 1, pparams->road[i].end(), Cell::Hole);
        nexhole[i] = (it == pparams->road[i].end()
                        ? std::numeric_limits<int>::max()
                        : std::distance(pparams->road[i].begin(), it));
    }
    bool no_hole = std::all_of(nexhole.begin(), nexhole.end(), [](auto h){
            h == std::numeric_limits<int>::max();
    });
    if (no_hole) {
        cands.push_back(Action::Speed);
        // Return {known_win=true, cands}
        return std::make_pair(true, cands);
    }

    static auto is_cand = [](Action a){
        return std::find(cands.begin(), cands.end(), a) != cands.end();
    };
    auto is_ok = [n_bikes=n_bikes(*pstate), min=pparams->min_bikes](int ndeath){
        return n_bikes - ndeath >= min;
    };
    std::array<bool, 4> deaths{ };
    std::array<int, 7> ndeaths;
    auto record_deaths = [&deaths, &ndeaths](Action a){
        return ndeaths[to_int(a)] = std::count(deaths.begin(), deaths.end(), true);
    };

    size_t npos = pstate->pos + pstate->speed;

    // Slow
    if (pstate->speed > 0) {
        for (int i=0; i<4; ++i) {
            deaths[i] = pstate->bikes[i] && (nexhole[i] < npos - 1);
        }
        if (is_ok(record_deaths(Action::Speed)))
            cands.push_back(Action::Slow);
    }
    // Wait
    if (is_cand(Action::Slow)) {
        for (int i=0; i<4; ++i) {
            deaths[i] = pstate->bikes[i] && (nexhole[i] <= npos);
        }
        if (is_ok(record_deaths(Action::Wait)))
            cands.push_back(Action::Wait);
    }
    // Speed
    if (is_cand(Action::Wait) && pstate->speed < 50) {
        for (int i=0; i<4; ++i) {
            deaths[i] = pstate->bikes[i] && (nexhole[i] <= npos + 1);
        }
        if (is_ok(record_deaths(Action::Speed)))
            cands.push_back(Action::Speed);
    }
    // Jump
    if (!is_cand(Action::Wait)) {
        for (int i=0; i<4; ++i) {
            deaths[i] = pstate->bikes[i] && (nexhole[i] == npos);
        }
        if (is_ok(record_deaths(Action::Jump)))
            cands.push_back(Action::Jump);
    }
    // Up
    if (!pstate->bikes[0]) {
        for (int i=1; i<4; ++i) {
            deaths[i] = pstate->bikes[i] && (nexhole[i] < npos || nexhole[i-1] <= npos);
        }
        if (is_ok(record_deaths(Action::Up)))
            cands.push_back(Action::Up);
    }
    // Down
    if (!pstate->bikes[3]) {
        for (int i=0; i<3; ++i) {
            deaths[i] = pstate->bikes[i] && (nexhole[i] < npos || nexhole[i+1] <= npos);
        }
        if (is_ok(record_deaths(Action::Down)))
            cands.push_back(Action::Down);
    }

    // Reverse the order of candidates in { Slow, Wait, Speed }
    if (cands.size() > 2 && cands[2] == Action::Speed) {
        std::swap(cands[0], cands[2]);
    } else if (cands.size() > 1 && cands[1] == Action::Wait) {
        std::swap(cands[0], cands[1]);
    }

    // Score actions by number of bikes preserved
    std::sort(cands.begin(), cands.end(), [&ndeaths](const auto a, const auto b) {
        return ndeaths[to_int(a)] > ndeaths[to_int(b)];
    });

    // Return {win_unknown, cands}
    return std::make_pair(false, cands);
}

// TODO: Save the results of next_holes dynamically for later usage.
inline void wait(State& s, GameParams::Road& r) {
    std::array<int, 4> nexhole = next_holes(s, r);
    for (int i=0; i<4; ++i) {
        s.bikes[i] &= nexhole[i] <= s.pos + s.speed;
    }
    s.pos += s.speed;
}

inline void slow(State& s, GameParams::Road& r) {
    --s.speed;
    wait(s, r);
}

inline void speed(State& s, GameParams::Road& r) {
    ++s.speed;
    wait(s, r);
}

inline void jump(State& s, GameParams::Road& r) {
    std::array<int, 4> nexhole = next_holes(s, r);
    for (int i=0; i<4; ++i) {
        s.bikes[i] &= nexhole[i] != s.pos + s.speed;
    }
    s.pos += s.speed;
}

inline void up(State& s, GameParams::Road& r) {
    if (s.bikes[0]) return;

    std::array<int, 4> nexhole = next_holes(s, r);
    for (int i=0; i<3; ++i) {
        s.bikes[i] = s.bikes[i+1]
            && nexhole[i+1] <= s.pos + s.speed
            && nexhole[i] < s.pos + s.speed;
    }
    s.bikes[3] = false;
    s.pos += s.speed;
}

inline void down(State& s, GameParams::Road& r) {
    if (s.bikes[3]) return;

    std::array<int, 4> nexhole = next_holes(s, r);
    for (int i=3; i>0; --i) {
        s.bikes[i] = s.bikes[i-1]
            && nexhole[i-1] <= s.pos + s.speed
            && nexhole[i] < s.pos + s.speed;
    }
    s.bikes[0] = false;
    s.pos += s.speed;
}

void Game::apply(const Action a, State& st) {
    using Cell = GameParams::Cell;
    st = *pstate;
    st.prev = pstate;
    pstate = &st;
    ++pstate->turn;

    switch(a) {
    case Action::Wait: return wait(*pstate, pparams->road);
    case Action::Slow: return slow(*pstate, pparams->road);
    case Action::Speed: return speed(*pstate, pparams->road);
    case Action::Jump: return jump(*pstate, pparams->road);
    case Action::Up: return up(*pstate, pparams->road);
    case Action::Down: return down(*pstate, pparams->road);
    default: return;
    }
}

void Game::undo() {
    assert(pstate->prev != nullptr);
    pstate = pstate->prev;
}

/**
 * Class that solves the problem.
 */
class Solver {
    enum class Status { Searching, DepthReached, Interrupted, Fail, Success };
    struct RootAction {
        explicit RootAction(Action a) : pv(1, a) {}
        std::vector<Action> pv;
        Status status{ Status::DepthReached };
        int depth_explored;
        int score{ std::numeric_limits<int>::min() };
        bool operator<(const RootAction& o);
        bool operator==(const RootAction& o) { return o.pv[0] == pv[0]; }
    };
    typedef std::vector<RootAction> RootActions;
    struct Stack {
        Action* pv;
        int depth;
        Action cur_action;
        Action excluded_action;
        int score;
    };
public:
    static void init(int time_per_turn_ms) { time = time_per_turn_ms; }
    Solver(Game& g) :
        game{g}
    {
    }
    Action get_next();
private:
    static int time;
    Game& game;
    RootActions ractions;
    std::array<Stack, Max_depth> stacks;
    std::vector<State> history;
    Status status;
    int explored_depth;
    int root_depth;

    void search();
    Status search(int depth, Stack* ss);
    bool time_up();
    void backtrack();
    void apply(const RootAction& ra);
};

Action Solver::get_next() {
    // Resume search and update pv if we haven't found a solution yet
    if (status != Status::Success)
        search();

    Action nex = ractions[0].pv[0];

    // Done with all the solution.
    if (nex == Action::None)
        return nex;

    ++root_depth;

    // Copy the rest of the pv to become the new pv, no need to update the state
    if (status == Status::Success) {
        std::copy(ractions[0].pv.begin()+1, ractions[0].pv.end(), ractions[0].pv.begin());
        // for (auto it = ractions[0].pv.begin(); it != ractions[0].pv.end() - 1; ++it)
        //     *it = *(it + 1);
    }

    // If we still haven't found a solution, update the game's state
    if (status != Status::Success) {
        State& st = history.emplace_back();
        game.apply(nex, st);
        --ractions[0].depth_explored;
    }

    return nex;
}

/**
 * Iterative deepening dfs
 */
void Solver::search() {
    ractions.clear();
    auto [win, cands] = game.candidates();

    // If no candidates
    if (cands.empty()) {
        ractions.push_back(RootAction(Action::None));
        status = Status::Fail;
    }

    // initialize the root actions.
    for (const Action cand : cands)
        ractions.push_back(RootAction(cand));

    // If there are no more holes ahead, we're done
    if (win) {
        status = Status::Success;
        while (!game.is_won()) {
            State st;
            game.apply(cands[0], st);
            ractions[0].pv.push_back(cands[0]);
            ractions[0].pv.push_back(Action::None);
        }
    }

    // Search the candidate moves
    for (int depth = 1; depth < Max_depth; ++depth) {
        stacks[0].depth = depth;
        stacks[0].excluded_action = Action::None;
        for (auto& ra : ractions) {
            stacks[0].cur_action = ra.pv[0];
            // Search with depth `depth` with stack corresponding to depth 0
            search(depth, &stacks[0]);
            ra.score = stacks[0].score;
        }

        std::stable_sort(ractions.begin(), ractions.end());

        if (time_up()) return;
    }
}

Solver::Status Solver::search(int depth, Stack* ss) {
    assert(game.turn() == root_depth);

    State st;
    game.apply(ss->cur_action, st);

    std::array<Action, 6> cands;
    auto [win, ccands] = game.candidates();

    Status ret = win
        ? Status::Success
        : ccands.empty() || game.is_lost()
        ? Status::Fail
        : depth == 0
        ? Status::DepthReached
        : Status::Searching;

    if (ret == Status::Searching)
    {
        ss->score = 2 * game.pos() - depth * (depth + 1);
        (ss+1)->depth = depth + 1;
        (ss+1)->excluded_action = Action::None;
        (ss+1)->score = std::numeric_limits<int>::min();
        std::copy(ccands.begin(), ccands.end(), cands.begin());

        for (const auto a : cands) {
            (ss+1)->cur_action = a;
            if ((ret = search(depth - 1, ss + 1)) == Status::DepthReached) {
                ss->score = (ss+1)->score > ss->score ? (ss+1)->score : ss->score;
                continue;
            } else {
                break;
            }
        }
    }

    if (ret == Status::DepthReached)
        ss->score = (ss+1)->score > ss->score ? (ss+1)->score : ss->score;
    else if (ret == Status::Success)
        ss->score = std::numeric_limits<int>::max();
    else if (ret == Status::Fail)
        ss->score = std::numeric_limits<int>::min();

    game.undo();
    // TODO: what else?
    return ret;
}

/**
 * Helper class representing either std::cin or some std::ifstream.
 */
class Istream {
public:
    Istream() = default;

    void set_file(const std::string& fn) {
        ifs = std::make_optional(std::ifstream{ fn });
    }

    operator std::istream&() {
        if (ifs) {
            return *ifs;
        } else {
            return std::cin;
        }
    }

    operator bool() {
        if (ifs) {
            return bool(*ifs);
        }
        return true;
    }

private:
    std::optional<std::ifstream> ifs;
};

int main(int argc, char *argv[])
{
    enum class Mode { Offline, Online };
    Mode mode = argc < 2 ? Mode::Online : Mode::Offline;
    Istream is;

    if (mode == Mode::Online)
        std::ios_base::sync_with_stdio(false);
    if (mode == Mode::Offline) {
        is.set_file(argv[1]);
        if (!is) {
            std::cerr << "Failed to open input file "
                  << argv[1] << std::endl;
            return EXIT_FAILURE;
        }
    }

    static constexpr int max_time_ms = 150;
    Game::init(is);
    Solver::init(max_time_ms);

    State init_state = input_turn(is);
    Game game;
    game.set(init_state);
    Solver solver(game);

    auto action = solver.get_next();

    while (action != Action::None) {
        std::cout << action << std::endl;
        input_turn_ignore(is);
        action = solver.get_next();
    }

    return 0;
}
