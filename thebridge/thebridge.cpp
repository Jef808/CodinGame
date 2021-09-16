#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Road {
    std::array<std::vector<int>, 4> holes;
    int length;
    void input(std::istream& _in);
    void show(std::ostream& _out);
};

void Road::input(std::istream& _in)
{
    std::string buf;
    for (int i=0; i<4; ++i) {
        std::getline(_in, buf);
        length = buf.size();
        for (int j=0; j<buf.size(); ++j)
            if (buf[j] == '0')
                holes[i].push_back(j);
    }
}

void Road::show(std::ostream& _out)
{
    for (int i=0; i<4; ++i) {
        for (const auto h : holes[i])
            _out << h << ' ';
        _out << '\n';
    }

}

struct Bikes {
    Bikes() :
        active{false},
        pos{0},
        speed{1}
    {}
    std::array<bool, 4> active;
    int nb_bikes;
    int pos;
    int speed;
    void input(std::istream& _in);
};

/** Update the bikes struct with the online info */
void Bikes::input(std::istream& _in)
{
    _in >> speed;
    int x, y, a;  // x-coord, y-coord, active-or-not
    for (int i=0; i<nb_bikes; ++i)
    {
        _in >> x >> y >> a;
        bool is_active = (active[y] = (a == 1));
        if (is_active)
            pos = x;
    }
}

/** Same as above but we discard the input */
inline void ignore_turn_input(std::istream& _in, int n_bikes)
{
    int s, x, y, a;
    _in >> s;
    for (int i=0; i<n_bikes; ++i)
        _in >> x >> y >> a;
}

bool operator==(const Bikes& a, const Bikes& b) {
    return a.active == b.active
        && a.pos == b.pos
        && a.speed == b.speed;
}

/**
 * Not meant as an interface but holds all
 * the settings and state of the problem.
 */
class Game
{
public:
    Game() = default;

    void initialize(std::istream&);
    void turn_input(std::istream&);
    void show(std::ostream&) const;
    int n_bikes() const { return m_bikes_nb; }
    int n_bikes_goal() const { return m_bikes_goal_nb; }
    bool lost() const;
    bool won() const;
    void speed();
    void slow();
    void up();
    void down();
    void jump();
    void wait();

private:
    Road m_road;
    Bikes m_bikes;
    int m_bikes_nb;
    int m_bikes_goal_nb;

public:
    bool operator==(const Game& other) {
        return m_bikes == other.m_bikes;
    }
};

/** Performs initial AND first turn input. */
void Game::initialize(std::istream &_in)
{
    _in >> m_bikes_nb >> m_bikes_goal_nb;
    _in.ignore();
    m_road.input(_in);
    m_bikes.nb_bikes = m_bikes_nb;
    m_bikes.input(_in);
}

inline void Game::turn_input(std::istream& _in)
{
    m_bikes.input(_in);
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

inline bool Game::lost() const
{
    return m_bikes_goal_nb < std::count(m_bikes.active.begin(), m_bikes.active.end(), true);
}

inline bool Game::won() const
{
    return m_bikes.pos >= m_road.length;
}

inline void Game::speed()
{
    ++m_bikes.speed;
    wait();
}

inline void Game::slow()
{
    --m_bikes.speed;
    wait();
}

inline void Game::up()
{
    if (m_bikes.active[0]) {
        wait(); return;
    }
    for (int i=0; i<3; ++i)
        m_bikes.active[i]
            = m_bikes.active[i+1];
    wait();
}

inline void Game::down()
{
    if (m_bikes.active[4]) {
        wait(); return;
    }
    for (int i=0; i<3; ++i)
        m_bikes.active[4-i]
            = m_bikes.active[3-i];
    wait();
}

inline void Game::jump()
{
    m_bikes.pos += m_bikes.speed;
    for (int i=0; i<4; ++i) {
        if (!m_bikes.active[i])
            continue;
        auto hole_it = std::find_if(m_road.holes[i].begin(), m_road.holes[i].end(),
          [p=m_bikes.pos](const auto h){
              return h >= p;
          });
        if (*hole_it == m_bikes.pos)
            m_bikes.active[i] = false;
    }
}

inline void Game::wait()
{
    int cur_pos = m_bikes.pos;
    m_bikes.pos = cur_pos + m_bikes.speed;
    for (int i=0; i<4; ++i) {
        if (!m_bikes.active[i])
            continue;
        bool bike_falls = false;
        auto hole_it = std::find_if(m_road.holes[i].begin(), m_road.holes[i].end(),
          [p = cur_pos](const auto h) {
              return h > p;
          });
        for (; hole_it != m_road.holes[i].end(); ++hole_it) {
            if (*hole_it <= m_bikes.pos)
                bike_falls = true;
        }
        if (bike_falls)
            m_bikes.active[i] = false;
    }
}

enum class Action {
    Speed, Jump, Up, Down, Slow, Wait
};

Action& operator++(Action& a) {
    if (a != Action::Wait)
        a = Action(static_cast<int>(a)+1);
    return a;
}

inline void simul(Game& game, const Action action)
{
    switch(action) {
        case Action::Wait: game.wait(); break;
        case Action::Speed: game.speed(); break;
        case Action::Slow: game.slow(); break;
        case Action::Up: game.up(); break;
        case Action::Down: game.down(); break;
        case Action::Jump: game.jump(); break;
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
    }
}


class Agent {
public:
    Agent() = default;
    void think(const Game& game);
    const std::deque<Action>& actions_seq() const;
private:
    std::deque<Action> m_actions;
    std::deque<Game> m_games;
};

void Agent::think(const Game& game)
{
    Action action = Action::Speed;
    m_games.push_back(game);

    bool done = false;
    for (int i=0; i<6; ++i) {
        Game _game = game;;
        simul(_game, Action(i));
        if (!_game.lost()) {

            m_games.push_back(_game);
            m_actions.push_back(Action(i));
        }
    }
}

const std::deque<Action>& Agent::actions_seq() const
{
    return m_actions;
}

int main(int argc, char *argv[])
{
    Game game{ };
    game.initialize(std::cin);

    Agent agent{ };
    auto actions = agent.actions_seq();

    while (!actions.empty()) {
        Action action = actions.back();
        actions.pop_back();
        std::cout << action << std::endl;
        ignore_turn_input(std::cin, game.n_bikes());
    }

    return EXIT_SUCCESS;
}
