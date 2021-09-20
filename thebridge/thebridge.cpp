#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <deque>
#include <iostream>
#include <iterator>
#include <fstream>
#include <numeric>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>


#include "viewer.h"

enum class Cell { Bridge, Hole };
std::ostream& operator<<(std::ostream& _out, const Cell);


// TODO Get rid of length (it's just there because the old one had one)
struct Road {
    using hole_iterator = std::vector<Cell>::const_iterator;

    std::array<std::vector<Cell>, 4> holes;
    /** Keep the position of the last holes on the bridge. */
    std::array<int, 4> last_holes;
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
};

enum class Action {
    None=0, Speed, Jump, Up, Down, Slow, Wait, Null=7
};
std::ostream& operator<<(std::ostream&, const Action);

/** To help with the State's constructor */
int cnt_bikes(const Bikes& bikes) {
    return std::count(bikes.active.begin(), bikes.active.end(), 1);
}

/// TODO: Get rid of the pointer to road somehow

struct State
{
    State(const Bikes& bikes, Road* road) :
        m_bikes{bikes}, m_road{road}, n_bks_left{cnt_bikes(bikes)}, n_turns{0}
    {
    }
    Bikes m_bikes;
    Road* m_road;
    int n_turns;
    int n_bks_left;

    void apply(const Action a);
    void speed();
    void slow();
    void up();
    void down();
    void jump();
    void wait();

    /**
     * Ret1: True if there is a whole between pos+1 and npos (can't speed or wait)
     * Ret2: True if npos has a hole (can't jump)
     */
    std::pair<bool, bool> holes_ahead(int pos, int npos, int lane) const;
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
    /** Update the bikes state from the game input. */
    void turn_input(std::istream& _in)
        { m_bikes.input(_in); }
    /** Read from the stream but discard the data. */
    void ignore_turn_input(std::istream& _in);
    /** the goal to achieve. */
    int n_bikes_goal() const
        { return n_bks_goal; }
    int road_length() const
        { return m_road.length; }
    /**
     * True if the state has no chance of winning.
     */
    bool lost(const State& s) const
        { return s.n_bks_left < n_bks_goal || s.n_turns > 50; }
    /**
     * True if didn't loose and the bikes are at the end of the bridge.
     */
    bool won(const State& s) const
        { return (s.m_bikes.pos >= m_road.length) && !lost(s); }
    /**
     * Smaller object for the Agent to manipulate.
     */
    State get_state()
        { return State{ m_bikes, &m_road }; }
    const Road& get_road() const
        { return m_road; }
     /**
     * Display some info.
     */
    void show(std::ostream& _out) const;

    /**
     * Display the road
     */
    void show_road(std::ostream& _out) const;
private:
    Road m_road;
    Bikes m_bikes;
    int n_bks;
    int n_bks_goal;
};

// TODO: Construct a class handling this efficiently.
std::pair<bool, bool> State::holes_ahead(int pos, int npos, int lane) const
{
    // Note, the second condition happens if a bike is as position 0 with speed 0!
    //if ((pos >= m_road->last_holes[lane]) || (pos == npos)) return std::make_pair(false, false);
    if (pos == npos) return std::make_pair(false, false);

    auto has_hole = std::find(m_road->begin(lane)+pos+1, m_road->begin(lane)+npos, Cell::Hole);

    return std::make_pair(has_hole != m_road->begin(lane)+npos,
                          m_road->holes[lane][npos] == Cell::Hole);
}

inline bool State::has_hole_at(int pos, int lane) const
{
    return m_road->begin(lane)[pos] == Cell::Hole;
}

// NOTE: The top lane has index 0 and bottom has index 3.
// When generating the candidate actions, we avoid UP,
// (resp DOWN) when the top (resp bottom) lanes are active.
//
//  0
//  1
//  2
//  3
//

inline void State::speed()
{
    m_bikes.speed += (m_bikes.speed < 50 ? 1 : 0);
    wait();
}

inline void State::slow()
{
    m_bikes.speed -= (m_bikes.speed > 0 ? 1 : 0);
    wait();
}

void State::up()
{
    for (int i=0; i<3; ++i){
        if (!m_bikes.active[i+1]) continue;

        n_bks_left = 0;
        auto holes_this_lane = holes_ahead(m_bikes.pos, m_bikes.pos + m_bikes.speed, i);
        auto holes_incoming_lane = holes_ahead(m_bikes.pos, m_bikes.pos + m_bikes.speed, i+1);
        m_bikes.active[i] = !(holes_this_lane.first
                              || holes_this_lane.second
                              || holes_incoming_lane.first);
    }
    m_bikes.pos += m_bikes.speed;
}

//TODO: Just treat two rows at a time instead of recomputing things
void State::down()
{
    for (int i=3; i>0; --i){
        if (!m_bikes.active[i-1]) continue;

        auto holes_this_lane = holes_ahead(m_bikes.pos, m_bikes.pos + m_bikes.speed, i);
        auto holes_incoming_lane = holes_ahead(m_bikes.pos, m_bikes.pos + m_bikes.speed, i-1);
        m_bikes.active[i] = !(holes_this_lane.first
                              || holes_this_lane.second
                              || holes_incoming_lane.first);
    }
    m_bikes.pos += m_bikes.speed;
}

inline void State::jump()
{
    int npos = m_bikes.pos + m_bikes.speed;
    for (int i=0; i<4; ++i) {
        m_bikes.active[i] &= !has_hole_at(npos, i);
    }
    m_bikes.pos = npos;
}

inline void State::wait()
{
   for (int i=0; i<4; ++i) {
       auto holes_data = holes_ahead(m_bikes.pos, m_bikes.pos + m_bikes.speed, i);
       m_bikes.active[i] &= !(holes_data.first || holes_data.second);
    }
    m_bikes.pos += m_bikes.speed;
}

/**
 * Tool to let the agent know when it's time to answer
 * even if it's not done.
 */
struct Stopwatch {
    Stopwatch() : m_start{std::chrono::steady_clock::now()}
    { }
    void reset() {
        m_start = std::chrono::steady_clock::now();
    }
    auto time() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_start).count();
    }
    std::chrono::steady_clock::time_point m_start;
};

class Agent {
public:

    Agent(Game& game) :
        m_game(game),
        m_actions_ndx{},
        m_states(1, game.get_state()),
        m_win{false},
        m_lost{false}
    {}

    bool solver() {
        m_sw.reset();
        return solve();
    }
    bool solve();
    Action nex_action();
    bool won() const { return m_win; }
    const std::deque<int>& actions_queue() const { return m_actions_ndx; }

private:
    Game& m_game;
    std::deque<int> m_actions_ndx;
    std::deque<State> m_states;
    std::deque<Action> winning_seq;
    bool m_win;
    bool m_lost;
    int m_timelim{ 450 };
    Stopwatch m_sw;

    bool out_of_time()
        { auto elapsed = m_sw.time();
            return (m_sw.time() > m_timelim); }
    void visit(const int ndx);
    void backtrack();
    std::array<int, 6> candidate_actions();
    const State& cur_state() const { return m_states.back(); }
};

inline void Agent::visit(const int a)
{
    const State& nex = m_states.back();
    m_states.push_back(nex);
    m_actions_ndx.push_back(a);
    m_states.back().apply(Action(a));
}

inline void Agent::backtrack()
{
    m_actions_ndx.pop_back();
    m_states.pop_back();
}

Action Agent::nex_action()
{
    //assert(!m_actions_ndx.empty());
    if (m_actions_ndx.empty()) return Action::Speed;

    auto a = m_actions_ndx.front();
    m_actions_ndx.pop_front();
    return Action(a);
}

// UP = 3, DOWN = 4.
constexpr  std::array<int, 6> no_updown = { 1, 2, 5, 6, 0, 0 };
constexpr  std::array<int, 6> no_up =     { 1, 2, 4, 5, 6, 0 };
constexpr  std::array<int, 6> no_down =   { 1, 2, 3, 5, 6, 0 };
constexpr  std::array<int, 6> all_acts =  { 1, 2, 3, 4, 5, 6 };

/**
 * For now this only gets rid of Up and Down when the
 * extremal lanes are occupied.
 */
std::array<int, 6> Agent::candidate_actions()
{
    std::array<int, 6> ret;
    const State& s = cur_state();

    bool top = s.m_bikes.active[0];   // Means can't go up
    bool bot = s.m_bikes.active[3];   // Means can't go down

    if (top) {
        if (bot) {
            ret = no_updown;
        } else {
            ret = no_up;
        }
    } else {
        if (bot) {
            ret = no_down;
        } else {
            ret = all_acts;
            // Mix up the order of UP/DOWN randomly
            if (int i = rand() % 2)
                std::swap(ret[2], ret[3]);
        }
    }

    return ret;
}

bool Agent::solve()
{
    std::tie(m_win, m_lost) = std::make_pair(m_game.won(cur_state()),
                                         m_game.lost(cur_state()));

    //cur_state().show(std::cerr);

    if (m_win) {
        std::cerr << "Found Win!\n************" << std::endl;

        // To inspect later... sometimes it's not quite winning...
        std::transform(m_actions_ndx.begin(), m_actions_ndx.end(), std::back_inserter(winning_seq),[](auto a){
            return Action(a);
        });

        return true;
    }

    if (!m_lost)
    {
        // We return true but the m_win member bool will still be off.
        // This way we can go back to main but come back here to continue the job.
        if (out_of_time()) return true;

        for (const auto a : candidate_actions())
        {
            if (a == 0) continue;

            //std::cerr << "Doing " << Action(a) << std::endl;

            // Note: we don't mark the bool m_win here because we could be returning
            // from the out_of_time check.
            visit(a);
            if (solve())
                return true;
        }
    }

    //std::cerr << "Backtracking...\n*******" << std::endl;

    backtrack();
    return false;
}

void Road::input(std::istream& _in)
{
    std::string buf;
    for (int i=0; i<4; ++i)
    {
        std::getline(_in, buf);
        std::transform(buf.begin(), buf.end(), std::back_inserter(holes[i]),
            [](const auto c){ return c == '0' ? Cell::Hole : Cell::Bridge; });
    }

    last_holes = { 0, 0, 0, 0 };
    for (int i=0; i<4; ++i) {
        auto last_hole = std::find(holes[i].rbegin(), holes[i].rend(), Cell::Hole);
        if (last_hole != holes[i].rend())
            last_holes[i] = holes[i].size() - std::distance(holes[i].rbegin(), last_hole);
    }
    length = holes[0].size();
}
/** Update the bikes struct with the online info */
void Bikes::input(std::istream& _in)
{
    std::stringstream ss{};
    int x, y, a;  // x-coord, y-coord, active-or-not
    _in >> speed; _in.ignore();

    ss << speed << '\n';

    for (int i=0; i<nb_bikes; ++i)
    {
        _in >> x >> y >> a; _in.ignore();

        ss << x << ' ' << y << ' ' << a << '\n';
        active[y] = (a == 1);
    }
    pos = x;
    std::cerr << ss.rdbuf() << std::endl;
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
    for (int i=0; i<4; ++i) {
        std::copy(holes[i].begin(), holes[i].end(), std::ostream_iterator<Cell>{ _out });
        _out << '\n';
    }
    _out.flush();
}

inline void Game::show_road(std::ostream& _out) const
{
    m_road.show(_out);
}
void State::apply(const Action action)
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
    n_bks_left = cnt_bikes(m_bikes);
}
std::ostream& operator<<(std::ostream& _out, const Action action)
{
    switch(action) {
        case Action::None: return _out << "NONE";
        case Action::Wait: return _out << "WAIT";
        case Action::Speed: return _out << "SPEED";
        case Action::Slow: return _out << "SLOW";
        case Action::Up: return _out << "UP";
        case Action::Down: return _out << "DOWN";
        case Action::Jump: return _out << "JUMP";
        default: return _out << "NULL";
    }
}
std::ostream& operator<<(std::ostream& _out, const Cell cell) {
    return _out << (cell == Cell::Hole ? '0' : '.');
}



enum class Mode { Local, Online };


int main(int argc, char *argv[])
{
    Mode mode;
    Game game;

    if (argc < 2)
    {
        std::cerr << "Online mode!" << std::endl;
        mode = Mode::Online;
        std::ios_base::sync_with_stdio(false);
        game.initial_input(std::cin);
    }
    else
    {
        std::cerr << "Local mode!" << std::endl;
        mode = Mode::Local;
        std::string fn = argv[1];
        std::ifstream ifs{ fn };
        if (!ifs) {
            std::cerr << "Failed to open input file "
                      << fn << std::endl;
            return EXIT_FAILURE;
        }
        game.initial_input(ifs);
    }

    Agent agent{ game };
    agent.solver();

    bool done = agent.won();

    if (mode == Mode::Online)
    {
        while (!done) {
            std::cout << agent.nex_action() << std::endl;
            game.ignore_turn_input(std::cin);
            agent.solver();
            done = agent.won();
        }

        while (true) {
            std::cout << agent.nex_action() << std::endl;
            game.ignore_turn_input(std::cin);
        }
    }

    else if (mode == Mode::Local)
    {
        int c = 0;
        while (!done && c <10) {
            agent.solver();
            done = agent.won();
            ++c;
        }

        State state = game.get_state();
        Road road = *state.m_road;
        Bikes bikes = state.m_bikes;

        const std::deque<int>& actions = agent.actions_queue();
        std::stringstream ss{};
        ss << "Agent has " << actions.size()
           << " actions in its queue.";
        int i=1;
        for (const auto a : actions) {
            ss << i << ": " << Action(a) << '\n';
            ++i;
        }
        i=0;
        std::string msg = ss.str();

        std::vector<Action> vactions;
        vactions.reserve(actions.size());
        std::transform(actions.begin(), actions.end(), std::back_inserter(vactions), [](const auto a){
            return Action(a);
        });

        ExtRoad xroad;
        xroad.init(road);
        xroad.update_bikes(bikes);
        xroad.show(std::cout);

        Viewer viewer{ xroad, state };
        viewer.view(std::cout, vactions.begin(), vactions.end(), msg);
    }

    return EXIT_SUCCESS;
}
