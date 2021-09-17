#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <deque>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <tuple>


struct Stopwatch
{
    using sc = std::chrono::steady_clock;
    Stopwatch() : m_start{ sc::now() } { }

    double operator()() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(sc::now() - m_start).count();
    }
    bool still_time(int time) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(sc::now() - m_start).count() < time;
    }
    void reset() {
        m_start = sc::now();
    }
    decltype(sc::now()) m_start;
};


struct Road
{
    using hole_iterator = std::vector<int>::const_iterator;

    std::array<std::vector<int>, 4> holes;
    int length;

    void input(std::istream& _in);
    void show(std::ostream& _out) const;

    hole_iterator begin(int i) const { return holes[i].begin(); }
    hole_iterator end(int i)   const { return holes[i].end();   }
};


struct Bikes
{
    Bikes() :
        active{0,0,0,0}, pos{0}, speed{1}
    {}

    std::array<bool, 4> active;
    int nb_bikes,
        pos,
        speed;

    void input(std::istream& _in);
    void turn_input(std::istream& _in);
};

enum class Action {
    None=0, Speed, Jump, Up, Down, Slow, Wait, Null=7
};
std::ostream& operator<<(std::ostream&, const Action);

/** To help with the State's constructor */
int cnt_bikes(const Bikes& bikes) {
    return std::count(bikes.active.begin(), bikes.active.end(), 1);
}

struct State
{
    State(const Bikes& bikes, Road* road) :
        m_bikes{bikes}, m_road{road}, n_bks_left{cnt_bikes(bikes)}, n_turns{0}
    {
    }
    Bikes m_bikes;
    Road* m_road;
    int n_bks_left;
    int n_turns;

    void apply(const Action a);
    void speed();
    void slow();
    void up();
    void down();
    void jump();
    void wait();

    /**
     * Checks if there is a hole ahead of current position,
     * up to the given end point.
     *
     * The first bool excludes the end point, the second one includes it.
     */
    std::pair<bool, bool> holes_ahead(int end, int lane) const;
    /**
     * Only checks for a hole at a prescribed position, for jumps
     */
    bool has_hole_at(int pos, int lane) const;
    void show(std::ostream& _out) const;

};

// To be deprecated...
class Game {
public:
    Game() = default;

    /** Performs initial AND first turn input. */
    void initial_input(std::istream&);
    /** Read from the stream but discard the data. */
    void ignore_turn_input(std::istream& _in);
    /** Update the bikes state from the game input. */
    void turn_input(std::istream& _in)
        { m_bikes.input(_in); }
    /**
     * True if the state has no chance of winning.
     */
    bool lost(const State& s) const
        { return s.n_bks_left < n_bks_goal || s.n_turns > 50; }
    /**
     * True if didn't loose and the bikes are at the end of the bridge.
     */
    bool won(const State& s) const
        { return (!lost(s)) && (s.m_bikes.pos >= m_road.length); }
    /**
     * Smaller object for the Agent to manipulate.
     */
    State get_state()
        { return State{ m_bikes, &m_road }; }
    int n_bikes() const
        { return n_bks; }
    int n_bikes_goal() const
        { return n_bks_goal; }
    int bike_pos() const
        { return m_bikes.pos; }
    void show_road(std::ostream& _out) const
        { m_road.show(_out); }
    void show(std::ostream&) const;
private:

    Road m_road;
    Bikes m_bikes;
    int n_bks;
    int n_bks_goal;
};

class Agent {
public:

    Agent(Game& game) :
        m_game(game),
        //m_actions{},
        m_actions_ndx{},
        m_states{1, game.get_state()},
        m_win{false},
        m_lost{false}
    {}
    // void init_state(const State& s)
    //     { m_states.push_back(s); }

    bool solve();
    Action nex_action();
    State get_root_state() const {
        State ret = m_game.get_state();
        return ret;
    }
    size_t n_actions() const { return m_actions_ndx.size(); }
    size_t road_length() const { return m_states.back().m_road->length; }
    const std::deque<int>& actions() const { return m_actions_ndx; }

private:
    Game& m_game;
    //std::deque<Action> m_actions;
    std::deque<int> m_actions_ndx;
    std::deque<State> m_states;
    bool m_win;
    bool m_lost;
    double m_time_limit { 80.0 };  // Have 150 ms per turn.
    Stopwatch sw{};

    /** Apply the action to the state using the */
    void visit(const int ndx);
    void visit(const Action a);
    void backtrack();
    const std::array<int, 6>& candidate_actions() const;
    const State& cur_state() const { return m_states.back(); }
    int cur_action_ndx() { return m_actions_ndx.back(); }
    Action cur_action() const { return Action{ m_actions_ndx.back() }; }
    bool out_of_time();

};

std::pair<bool, bool> State::holes_ahead(int end, int lane) const
{
    // Check if the next hole is before or after the new pos
    auto nex_hole = std::find_if(m_road->begin(lane), m_road->end(lane),
                                 [p=m_bikes.pos](auto h_i){ return p < h_i; });

    // Excluding the end-point:
    bool no_holes_exc = (
        (nex_hole == m_road->end(lane)) || (*nex_hole >= end));
    // Including it:
    bool no_holes_inc = no_holes_exc && (*nex_hole > end);

    return std::make_pair(!no_holes_exc, !no_holes_inc);
}

inline bool State::has_hole_at(int pos, int lane) const
{
    return std::find(m_road->begin(lane), m_road->end(lane), pos) != m_road->end(lane);
}

inline void State::speed()
{
    ++m_bikes.speed;
    wait();
}

inline void State::slow()
{
    m_bikes.speed -= (m_bikes.speed > 0 ? 1 : 0);
    wait();
}

void State::up()
{
    // 0 corresponds to the top lane:
    // If there is a bike in the upper lane, this just becomes a wait.
    // if (m_bikes.active[0]) {
    //     wait();
    //     return;
    // }
    int dest_pos = m_bikes.pos + m_bikes.speed;

    std::array<std::pair<bool, bool>, 3> holes_ahead_dat {
        holes_ahead(dest_pos, 1),
        holes_ahead(dest_pos, 2),
        holes_ahead(dest_pos, 3)
    };

    // For the bottom 3 lanes, set the status of the bike in the lane
    // right above it for the next turn:
    for (int i=0; i<3; ++i)
    {
        n_bks_left -= (
            !(m_bikes.active[i] = (
                  // There was a bike below, no holes below (exclusive range), no holes
                  // in current lane (inclusive range)
                  (m_bikes.active[i+1] && !holes_ahead_dat[i+1].first && !holes_ahead_dat[i].second))));
    }

    m_bikes.pos = dest_pos;
}

void State::down()
{
    // 3 corresponds to the bottom lane:
    // If there is a bike in the bottom lane, this just becomes a wait.
    if (m_bikes.active[3]) {
        wait();
        return;
    }
    int dest_pos = m_bikes.pos + m_bikes.speed;

    std::array<std::pair<bool, bool>, 3> holes_ahead_dat {
    holes_ahead(dest_pos, 2),
    holes_ahead(dest_pos, 1),
    holes_ahead(dest_pos, 0)
    };

    // For the bottom 3 lanes, set the status of the bike in the lane
    // right above it for the next turn:
    for (int i=3; i>0; --i)
    {
        n_bks_left -= (
            !(m_bikes.active[i] = (
                  // There was a bike below, no holes below (exclusive range), no holes
                  // in current lane (inclusive range)
                  (m_bikes.active[i-1] && !holes_ahead_dat[i-1].first && !holes_ahead_dat[i].second))));
    }

    m_bikes.pos = dest_pos;
}

inline void State::jump()
{
    int dest_pos = m_bikes.pos + m_bikes.speed;

    for (int i=0; i<4; ++i) {
        if (!m_bikes.active[i]) continue;

        n_bks_left -= (
            !(m_bikes.active[i] = !has_hole_at(dest_pos, i)));
    }
    m_bikes.pos = dest_pos;
}
inline void State::wait()
{
    int dest_pos = m_bikes.pos + m_bikes.speed;

    for (int i=0; i<4; ++i) {
        if (!m_bikes.active[i]) continue;

        n_bks_left -= (
            !(m_bikes.active[i] = !holes_ahead(dest_pos, i).first));
    }
    m_bikes.pos = dest_pos;
}

inline void Agent::visit(const int a)
{
    const State& nex = m_states.back();
    m_states.push_back(nex);
    m_states.back().apply(Action{ a });
    m_actions_ndx.push_back(a);
}

inline void Agent::backtrack()
{
    //m_actions.pop_back();
    m_actions_ndx.pop_back();
    m_states.pop_back();
}

// Action Agent::nex_action()
// {
//     if (m_actions_ndx.empty()) return Action::Wait;

//     int ndx = m_actions_ndx.front();
//     m_actions_ndx.pop_front();

//     return Action{ ndx };
// }

Action Agent::nex_action()
{
    if (m_actions_ndx.empty()) return Action::Jump;

    auto a = Action{ m_actions_ndx.front() };
    m_actions_ndx.pop_front();
    return a;
}

void play_simul(const State& _state, const std::deque<int>& _actions)
{
    State state = _state;
    auto actions = _actions;
    while (!actions.empty())
    {
        state.apply(Action{ actions.front() });
        actions.pop_front();
        state.show(std::cerr);
    }
}

static bool played_simul = false;
constexpr  std::array<int, 6> no_updown = { 1, 2, 5, 6, 0, 0 };
constexpr  std::array<int, 6> no_up =     { 1, 2, 4, 5, 6, 0 };
constexpr  std::array<int, 6> no_down =   { 1, 2, 3, 5, 6, 0 };
constexpr  std::array<int, 6> all_acts =  { 1, 2, 3, 4, 5, 6 };

const std::array<int, 6>& Agent::candidate_actions() const {
    const auto& s = cur_state();

    return (!s.m_bikes.active[0]
            ? (!s.m_bikes.active[3]
               ? all_acts
               : no_down)
            : (!s.m_bikes.active[3]
               ? no_up
               : no_updown));

    // Can improve lots here...
    // - e.g. Prune for speed given the distance to the end,
    // - e.g. Also given number of turns left
    // - e.g. Need to match certain speeds to jump accross wide holes

}

bool Agent::solve()
{
    std::tie(m_win, m_lost) = std::tuple(m_game.won(cur_state()),
                                         m_game.lost(cur_state()));

    if (m_win) {
        // if (!played_simul) {
        //     std::cerr << "Found win!" << std::endl;
        //     play_simul(m_states.front(), m_actions_ndx);
        //     played_simul = true;
        // }
        std::cerr << "Found win!" << std::endl;
        return true;
    }
    if (!m_lost)
    {
        //for (const auto a : {1, 2, 3, 4, 5, 6 })
        for (const auto a : candidate_actions())
        {
            if (a == 0) continue;

            visit(a);
            if (solve())
                return true;
        }
    }
    backtrack();
    return false;
}

void Road::input(std::istream& _in)
{
    std::string buf;
    for (int i=0; i<4; ++i)
    {
        std::getline(_in, buf);
        for (int j=0; j<buf.size(); ++j)
        {
            if (buf[j] == '0')
                holes[i].push_back(j);
        }
    }
    length = buf.size();
}
/** Update the bikes struct with the online info */
void Bikes::input(std::istream& _in)
{
    int x, y, a;  // x-coord, y-coord, active-or-not
    _in >> speed; _in.ignore();

    for (int i=0; i<nb_bikes; ++i)
    {
        _in >> x >> y >> a; _in.ignore();
        active[y] = (a == 1);
    }

    pos = x;
}
void Game::initial_input(std::istream &_in)
{
    _in >> n_bks
        >> n_bks_goal; _in.ignore();

    m_road.input(_in);
    m_bikes.nb_bikes = n_bks;
    m_bikes.input(_in);
}
inline void Game::ignore_turn_input(std::istream& _in)
{
    std::string buf;
    for (int i=0; i<n_bks+1; ++i)
        std::getline(_in, buf);
}
void Road::show(std::ostream& _out) const
{
    for (int i=0; i<4; ++i)
    {
        for (int j=0; j<length; ++j)
        {
_out << (std::find(holes[i].begin(), holes[i].end(), j)
         == holes[i].end() ? '-' : 'O');
        }
        _out << '\n';
    }
    _out << std::endl;
}
void State::show(std::ostream& _out) const
{
    _out << "Speed: " << m_bikes.speed
         << "\nPos: " << m_bikes.pos << '\n'
         << "n_bikes: " << n_bks_left << '\n';

    for (int i=0; i<4; ++i) {
        _out << (m_bikes.active[i] ? 'Y' : 'N')
             << '\n';
    }
    _out << std::endl;
}
// void Game::show(std::ostream& _out) const
// {
//     _out << "Speed: " << m_bikes.speed
//          << "\nPos: " << m_bikes.pos
//          << '\n';

//     for (int i=0; i<4; ++i) {
//         _out << (m_bikes.active[i] ? 'Y' : 'N')
//              << '\n';
//     }
//     _out << std::endl;
// }
inline void State::apply(const Action action)
{
    ++n_turns;
    switch(action) {
        case Action::Wait: wait(); break;
        case Action::Speed: speed(); break;
        case Action::Slow: slow(); break;
        case Action::Up: up(); break;
        case Action::Down: down(); break;
        case Action::Jump: jump(); break;
        default: return;
    }
}
inline std::ostream& operator<<(std::ostream& _out, const Action action)
{
    switch(action) {
        case Action::None: return _out << "NONE";
        case Action::Wait: return _out << "WAIT";
        case Action::Speed: return _out << "SPEED";
        case Action::Slow: return _out << "SLOW";
        case Action::Up: return _out << "UP";
        case Action::Down: return _out << "DOWN";
        case Action::Jump: return _out << "JUMP";
        case Action::Null: return _out << "NULL";
    }
}

struct Simul_debug {
    Simul_debug(const Agent& agent) :
        m_state{agent.get_root_state()}, m_actions_ndx{agent.actions()}
    {}

    void operator()(std::istream& _in, std::ostream& _out)
    {
        if (m_actions_ndx.empty()) {
            m_state.apply(Action::Wait);
        } else {
            m_state.apply(Action{ m_actions_ndx.front() });
            m_actions_ndx.pop_front();
        }

        _out << "Simulated bikes:" << std::endl;
        m_state.show(_out);

        m_state.m_bikes.input(_in);

        _out << "Actual bikes:" << std::endl;
        m_state.show(_out);
    }

    State m_state;
    std::deque<int> m_actions_ndx;
};

int main(int argc, char *argv[])
{
    std::ios_base::sync_with_stdio(false);


    Game game{ };
    game.initial_input(std::cin);

    Agent agent{ game };

    bool done = agent.solve();
    assert(done);

    // For debugging
    //Simul_debug debug_turn{ agent };

    while (true)
    {
        std::cout << agent.nex_action() << std::endl;
        //assert(a != Action::None);

        //debug_turn(std::cin, std::cerr);
        game.ignore_turn_input(std::cin);

    }

    return EXIT_SUCCESS;
}
