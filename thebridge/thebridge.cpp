#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <deque>
#include <iostream>
#include <numeric>
#include <string>
#include <random>
#include <vector>
#include <tuple>


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

enum class Cell { Bridge, Hole };

struct Roadv2 {
    using hole_iterator = std::vector<Cell>::const_iterator;

    /** Store every piece of the bridge for faster lookups */
    std::array<std::vector<Cell>, 4> holes;
    /** Keep the position of the last holes on the bridge. */
    std::array<int, 4> last_holes;

    void input(std::istream& _in);
    void show(std::ostream& _out) const;

    hole_iterator begin(int i) const { return holes[i].begin(); }
    hole_iterator end(int i)   const { return holes[i].end();   }
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
    Roadv2* m_roadv2;
    int n_turns;
    int n_bks_left;

    void set_roadv2(Roadv2* road2) { m_roadv2 = road2; }

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
    //std::pair<bool, bool> holes_ahead(const Roadv2& r, int pos, int npos, int lane) const;
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
     /**
     * Display some info.
     */
    void show(std::ostream& _out) const;

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
        m_actions_ndx{},
        m_states{1, game.get_state()},
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
    //const std::deque<int>& actions() const { return m_actions_ndx; }

private:
    Game& m_game;
    std::deque<int> m_actions_ndx;
    std::deque<State> m_states;
    bool m_win;
    bool m_lost;
    int m_timelim{ 130 };
    Stopwatch m_sw;

    bool out_of_time() {
        auto elapsed = m_sw.time();
        // std::cerr << elapsed << std::endl;
        return (elapsed > m_timelim);
    }
    void visit(const int ndx);
    void backtrack();
    std::array<int, 6> candidate_actions();
    const State& cur_state() const { return m_states.back(); }
};

// NOTE: Defining no_holes_excl first then no_holes_incl, you run into
// problems when pos = 0 and there are no hole along the lane...
std::pair<bool, bool> State::holes_ahead(int nex_pos, int lane) const
{
    // Find the first hole that's past the current bikes' position
    auto nex_hole = std::find_if(m_road->begin(lane), m_road->end(lane),
                                 [p=m_bikes.pos](auto h_i){ return p < h_i; });

    // Checking if we're good up to the end point)
    bool no_holes_incl = (nex_hole == m_road->end(lane) || (*nex_hole > nex_pos));

    // Relaxing the check to allow for a hole at the end-point
    bool no_holes_excl = (no_holes_incl || (*nex_hole == nex_pos));

    return std::make_pair(!no_holes_excl, !no_holes_incl);
}

inline bool State::has_hole_at(int pos, int lane) const
{
    return std::find(m_road->begin(lane), m_road->end(lane), pos) != m_road->end(lane);
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
    int dest_pos = m_bikes.pos + m_bikes.speed;

    std::array<std::pair<bool, bool>, 4> holes_ahead_dat {
        holes_ahead(dest_pos, 0),
        holes_ahead(dest_pos, 1),
        holes_ahead(dest_pos, 2),
        holes_ahead(dest_pos, 3)
    };
    n_bks_left = 0;

    for (int i=0; i<3; ++i){
    // Check that the incoming lane (below) was active and the bike didn't fall.
    n_bks_left += (m_bikes.active[i] = (m_bikes.active[i+1]
                    && !(holes_ahead_dat[i+1].first || holes_ahead_dat[i].second)));
    }

    m_bikes.pos = dest_pos;
}

void State::down()
{
    int dest_pos = m_bikes.pos + m_bikes.speed;

    std::array<std::pair<bool, bool>, 4> holes_ahead_dat {
        holes_ahead(dest_pos, 0),
        holes_ahead(dest_pos, 1),
        holes_ahead(dest_pos, 2),
        holes_ahead(dest_pos, 3)
    };
    n_bks_left = 0;

    for (int i=3; i>0; --i){
    n_bks_left += (m_bikes.active[i] = (m_bikes.active[i-1]
                    && !(holes_ahead_dat[i-1].first || holes_ahead_dat[i].second)));
    }

    m_bikes.pos = dest_pos;
}

inline void State::jump()
{
    int dest_pos = m_bikes.pos + m_bikes.speed;
    n_bks_left = 0;

    for (int i=0; i<4; ++i) {
        n_bks_left += (m_bikes.active[i] &= !(has_hole_at(dest_pos, i)));
    }
    m_bikes.pos = dest_pos;
}

inline void State::wait()
{
   int dest_pos = m_bikes.pos + m_bikes.speed;
   n_bks_left = 0;
   for (int i=0; i<4; ++i) {
         n_bks_left += (m_bikes.active[i] &= !(holes_ahead(dest_pos, i).second));
    }
     m_bikes.pos = dest_pos;
}

inline void Agent::visit(const int a)
{
    const State& nex = m_states.back();
    m_states.push_back(nex);
    m_actions_ndx.push_back(a);

    m_states.back().apply(Action{ a });
}

inline void Agent::backtrack()
{
    m_actions_ndx.pop_back();
    m_states.pop_back();
}

Action Agent::nex_action()
{
    if (m_actions_ndx.empty()) return Action::Speed;

    auto a = m_actions_ndx.front();
    m_actions_ndx.pop_front();
    return Action{ a };
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
    using uid = std::uniform_int_distribution<int>;

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
    std::tie(m_win, m_lost) = std::tuple(m_game.won(cur_state()),
                                         m_game.lost(cur_state()));

    //cur_state().show(std::cerr);

    if (m_win) {
        //std::cerr << "Found Win!\n************" << std::endl;
        return true;
    }

    if (!m_lost)
    {
        // We return true but the m_win member bool will still be off.
        // This way we can go back to main but come back here to continue the job.

        if (out_of_time()) return true;

        for (const auto a : candidate_actions())
        {

            // We pad the std::array with zeros instead of allocating vectors so
            // there are some bad values
            if (a == 0) continue;

            //std::cerr << "Doing " << Action{ a } << std::endl;

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
        for (int j=0; j<buf.size(); ++j)
        {
            if (buf[j] == '0')
                holes[i].push_back(j);
        }
    }
    length = buf.size();
}
void Roadv2::input(std::istream& _in)
{
    std::string buf;
    for (int i=0; i<4; ++i)
    {
        std::getline(_in, buf);
        std::transform(buf.begin(), buf.end(), std::back_inserter(holes[i]),
            [](const auto c){ return c == '-' ? Cell::Bridge : Cell::Hole; });
    }
    last_holes = { 0, 0, 0, 0 };
    for (int i=0; i<4; ++i) {
        auto last_hole = std::find(holes[i].rbegin(), holes[i].rend(), Cell::Hole);
        if (last_hole != holes[i].rend())
            last_holes[i] = holes[i].size() - std::distance(holes[i].rbegin(), last_hole);
    }
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
        auto nex_hole_it = holes[i].begin();
        for (int j=0; j<length; ++j)
        {
            if (j == *nex_hole_it){
                ++nex_hole_it;
                _out << '0';
            } else {
                _out << '-';
            }
        }
        _out << std::endl;
    }
}
void State::show(std::ostream& _out) const
{
    _out << "Speed: " << m_bikes.speed
         << "\nPos: " << m_bikes.pos
         << "\nn_bikes_left: " << n_bks_left << std::endl;

    for (int i=0; i<4; ++i) {
        _out << (m_bikes.active[i] ? 'Y' : 'N') << std::endl;
    }
}
void Game::show(std::ostream& _out) const
{
    _out << "Speed: " << m_bikes.speed
         << "\nPos: " << m_bikes.pos << std::endl;

    for (int i=0; i<4; ++i) {
        _out << (m_bikes.active[i] ? 'Y' : 'N') << std::endl;
    }
}

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


int main(int argc, char *argv[])
{
    std::ios_base::sync_with_stdio(false);

    Game game{ };
    game.initial_input(std::cin);

    Agent agent{ game };
    agent.solver();
    bool done = agent.won();

    // Let the agent go back to think if doesn't have a winning
    // path after the first timelimit is attained.
    while (!done) {
        std::cout << agent.nex_action() << std::endl;
        game.ignore_turn_input(std::cin);
        agent.solver();
        done = agent.won();
    }

    // Once done == true, we know the action queue contains a
    // winning sequence.
    while (true)
    {
        std::cout << agent.nex_action() << std::endl;

        //debug_turn(std::cin, std::cerr);
        game.ignore_turn_input(std::cin);
        //game.turn_input(std::cin);
        //game.show(std::cerr);
    }

    return EXIT_SUCCESS;
}
