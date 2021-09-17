#include <algorithm>
#include <array>
#include <chrono>
#include <deque>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>


struct Stopwatch
{
    using sc = std::chrono::steady_clock;
    Stopwatch() :
        m_start{ sc::now() }
    {}

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
};


struct State
{
    Bikes m_bikes;
    Road* m_road;
    int n_bks_left;

    void speed();
    void slow();
    void up();
    void down();
    void jump();
    void wait();

    void show(std::ostream& _out) const;
};


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
        { return s.n_bks_left < n_bks_goal; }
    /**
     * True if didn't loose and the bikes are at the end of the bridge.
     */
    bool won(const State& s) const
        { return !(lost(s)) && s.m_bikes.pos >= m_road.length; }
    /**
     * Smaller object for the Agent to manipulate.
     */
    State get_state()
        { return State{ m_bikes, &m_road }; }

    int n_bikes() const
        { return n_bks; }
    int n_bikes_goal() const
        { return n_bks_goal; }
    void show_road(std::ostream& _out) const
        { m_road.show(_out); }
    void show(std::ostream&) const;
private:

    Road m_road;
    Bikes m_bikes;
    int n_bks;
    int n_bks_goal;
};

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

inline void State::up()
{
    if (!m_bikes.active[0])
    {
        for (int i=0; i<3; ++i)
        m_bikes.active[i] = m_bikes.active[i+1];
    }
    wait();
}

inline void State::down()
{
    if (!m_bikes.active[4])
    {
        for (int i=0; i<3; ++i)
        m_bikes.active[4-i] = m_bikes.active[3-i];
    }
    wait();
}

inline void State::jump()
{
    m_bikes.pos += m_bikes.speed;
    for (int i=0; i<4; ++i) {
    // Only die if new pos is on a hole
    n_bks_left -= (
    (m_bikes.active[i] =
         std::find(m_road->begin(i), m_road->end(i), m_bikes.pos)
             != m_road->end(i)));
    }
}
inline void State::wait()
{
    int dest_pos = m_bikes.pos + m_bikes.speed;

    for (int i=0; i<4; ++i)
    {
        if (!m_bikes.active[i]) continue;

    // Check if the next hole is before or after the new pos
    auto nex_hole = std::find_if(m_road->begin(i), m_road->end(i),
        [p=m_bikes.pos](auto h_i){ return p < h_i; });

    n_bks_left -= (
        (m_bikes.active[i] =
         (nex_hole != m_road->end(i)) && (*nex_hole > dest_pos)));
    }
    m_bikes.pos = dest_pos;
}

enum class Action {
    None, Speed, Jump, Up, Down, Slow, Wait, Null=7
};

class Agent {
public:
    using Action_index = uint8_t;

    Agent(Game& game) :
        m_game(game)
    {}
    void init_state(const State& s)
        { m_states.push_back(s); }

    bool solve();
    Action nex_action();
    void set_timed_output(std::ostream&, int);

private:
    Game& m_game;
    std::deque<int> m_actions_ndx;
    std::deque<State> m_states;
    bool m_win;
    bool m_lost;
    double m_time_limit { 80.0 };  // Have 150 ms per turn.
    Stopwatch sw{};

    void visit(const Action);
    void backtrack();
    const State& cur_state() const { return m_states.back(); }
    int cur_action_ndx() { return m_actions_ndx.back(); }
    Action cur_action() const { return Action{ m_actions_ndx.back() }; }
    bool out_of_time();

};

void simul(State&, const Action, const Game&);

inline void Agent::visit(const Action a)
{
    State nex = m_states.back();
    m_states.push_back(std::move(nex));

    simul(nex, a, m_game);
}

inline void Agent::backtrack()
{
    m_actions_ndx.pop_back();
    m_states.pop_back();
}

Action Agent::nex_action()
{
    auto ndx = m_actions_ndx.front();
    m_actions_ndx.pop_front();
    return Action{ ndx };
}

bool Agent::solve()
{
    std::tie(m_win, m_lost) = std::tuple(m_game.won(cur_state()),
                                         m_game.lost(cur_state()));
    if (m_win) return true;
    if (!m_lost)
    {
        for (int n=1; n<7; ++n)
        {
            visit(Action{ n });
            if (solve())
                return (m_win = true);
        }
    }
    backtrack();
    return false;
}

std::ostream& operator<<(std::ostream&, const Action);



int main(int argc, char *argv[])
{
    std::ios_base::sync_with_stdio(false);


    Game game{ };
    game.initial_input(std::cin);

    State state = game.get_state();

    Agent agent{ game };
    agent.init_state(state);

    while (true)
    {
        std::cout << agent.nex_action() << std::endl;
        game.ignore_turn_input(std::cin);
    }

    return EXIT_SUCCESS;
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

inline void ignore_turn_input(std::istream& _in, int n_bikes)
{
    std::string buf;
    for (int i=0; i<n_bikes+1; ++i)
        std::getline(_in, buf);
}

void Game::show(std::ostream& _out) const
{
    _out << "Speed: " << m_bikes.speed
         << "\nPos: " << m_bikes.pos
         << '\n';
    for (int i=0; i<4; ++i) {
        _out << (m_bikes.active[i] ? 'Y' : 'N')
             << '\n';
    }
    _out << std::endl;
}

void State::show(std::ostream& _out) const
{
    _out << "Speed: " << m_bikes.speed
         << "\nPos: " << m_bikes.pos << '\n';

    for (int i=0; i<4; ++i) {
        _out << (m_bikes.active[i] ? 'Y' : 'N')
             << '\n';
    }
    _out << std::endl;
}

inline void simul(State& state, const Action action, const Game& game)
{
    switch(action) {
        case Action::Wait: state.wait(); break;
        case Action::Speed: state.speed(); break;
        case Action::Slow: state.slow(); break;
        case Action::Up: state.up(); break;
        case Action::Down: state.down(); break;
        case Action::Jump: state.jump(); break;
        default: return;
    }
}

inline std::ostream& operator<<(std::ostream& _out, const Action action)
{
    switch(action) {
        case Action::Wait: return _out << "WAIT";
        case Action::Speed: return _out << "SPEED";
        case Action::Slow: return _out << "SLOW";
        case Action::Up: return _out << "UP";
        case Action::Down: return _out << "DOWN";
        case Action::Jump: return _out << "JUMP";
        default: return _out << "NOTHING";
    }
}
