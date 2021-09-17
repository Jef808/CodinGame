#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <tuple>


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
    /**
     * Display the parameters of the state.
     */
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

    bool solve();
    Action nex_action();
    //const std::deque<int>& actions() const { return m_actions_ndx; }

private:
    Game& m_game;
    std::deque<int> m_actions_ndx;
    std::deque<State> m_states;
    bool m_win;
    bool m_lost;

    void visit(const int ndx);
    void backtrack();
    const std::array<int, 6>& candidate_actions() const;
    const State& cur_state() const { return m_states.back(); }
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
    bool no_holes_inc = no_holes_exc && (*nex_hole != end);

    return std::make_pair(!no_holes_exc, !no_holes_inc);
}

inline bool State::has_hole_at(int pos, int lane) const
{
    return std::find(m_road->begin(lane), m_road->end(lane), pos) != m_road->end(lane);
}

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
    // NOTE: This is important but we check this in candidate_actions()
    // if (m_bikes.active[0]) {
    //     wait();
    //     return;
    // }
    int dest_pos = m_bikes.pos + m_bikes.speed;

    std::array<std::pair<bool, bool>, 4> holes_ahead_dat {
        holes_ahead(dest_pos, 0),
        holes_ahead(dest_pos, 1),
        holes_ahead(dest_pos, 2),
        holes_ahead(dest_pos, 3)
    };

    // For the bottom 3 lanes, set the status of the bike in the lane
    // right above it for the next turn:
    for (int i=0; i<3; ++i)
    {
        bool incoming = m_bikes.active[i+1];
        if (!incoming)
        {
            m_bikes.active[i] = false;
            continue;
        }

        bool fall = (holes_ahead_dat[i+1].first || holes_ahead_dat[i].second);
        // since we're going up in indices...
        if (fall) {
            --n_bks_left;
            m_bikes.active[i] = false;
        }

        m_bikes.active[i] = true;
    }
    m_bikes.pos = dest_pos;
}

void State::down()
{
    // 3 corresponds to the bottom lane:
    // If there is a bike in the bottom lane, this just becomes a wait.
    //
    // NOTE: This is important but we check this in candidate_actions()
    // if (m_bikes.active[3]) {
    //     wait();
    //     return;
    // }
    int dest_pos = m_bikes.pos + m_bikes.speed;

    std::array<std::pair<bool, bool>, 4> holes_ahead_dat {
    holes_ahead(dest_pos, 0),
    holes_ahead(dest_pos, 1),
    holes_ahead(dest_pos, 2),
    holes_ahead(dest_pos, 3)
    };

    // For the bottom 3 lanes, set the status of the bike in the lane
    // right above it for the next turn:
    for (int i=3; i>0; --i)
    {
        bool incoming = m_bikes.active[i-1];
        if (!incoming)
        {
            m_bikes.active[i] = false;
            continue;
        }

        bool fall = (holes_ahead_dat[i-1].first || holes_ahead_dat[i].second);
        // since we're going up in indices...
        if (fall) {
            --n_bks_left;
            m_bikes.active[i] = false;
        }

        m_bikes.active[i] = true;
        //NOTE: We need to make sure nobody is in the third lane at first for this
        // to work, and do it in this order!
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
            !(m_bikes.active[i] = !holes_ahead(dest_pos, i).second));
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
    m_actions_ndx.pop_back();
    m_states.pop_back();
}

Action Agent::nex_action()
{
    auto a = m_actions_ndx.empty()
        ? Action::Wait
        : Action{ m_actions_ndx.front() };
    m_actions_ndx.pop_front();
    return a;
}

constexpr  std::array<int, 6> no_updown = { 1, 2, 5, 6, 0, 0 };
constexpr  std::array<int, 6> no_up =     { 1, 2, 4, 5, 6, 0 };
constexpr  std::array<int, 6> no_down =   { 1, 2, 3, 5, 6, 0 };
constexpr  std::array<int, 6> all_acts =  { 1, 2, 3, 4, 5, 6 };

// enum class Action {
//     None=0, Speed=1, Jump=2, Up=3, Down=4, Slow=5, Wait=6, Null=7
// };


// NOTE: It would be much more natural to have a helper class
// keeping track of the holes ahead of the bikes...
const std::array<int, 6>& Agent::candidate_actions() const
{
    const State& s = cur_state();

    // The top lane is occupied
    bool top = s.m_bikes.active[0];
    bool bot = s.m_bikes.active[3];

    if (top) {
        if (bot) {
            return all_acts;
        } else {
            return no_down;
        }
    } else {
        if (bot) {
            return no_up;
        } else {
            return no_updown;
        }
    }

    // Could maybe help here with:
    // - e.g. Prune for speed given the distance to the end,
    // - e.g. Also given number of turns left
    // - e.g. Need to match certain speeds to jump accross wide holes

    // int n_surplus = m_game.n_bikes_goal() - s.n_bks_left;
    // int n_turns_left = 50 - s.n_turns;
    // int dist_to_end = m_game.road_length() - s.m_bikes.pos;
}

bool Agent::solve()
{
    std::tie(m_win, m_lost) = std::tuple(m_game.won(cur_state()),
                                         m_game.lost(cur_state()));

    if (m_win) {
        //std::cerr << "Found win!" << std::endl;
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
         << "n_bikes_left: " << n_bks_left << '\n';

    for (int i=0; i<4; ++i) {
        _out << (m_bikes.active[i] ? 'Y' : 'N')
             << '\n';
    }
    _out << std::endl;
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

struct Simul_debug {
    Simul_debug(const State& state, const std::deque<int>& actions) :
        m_state{state}, m_actions_ndx{actions}
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

        //debug_turn(std::cin, std::cerr);
        game.ignore_turn_input(std::cin);

    }

    return EXIT_SUCCESS;
}
