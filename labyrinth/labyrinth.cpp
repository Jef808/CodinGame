#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

enum class Cell {
    Empty = 0,
    Wall,
    Start,
    Target,
    Unknown,
};

enum class Action {
    None = 0,
    Left,
    Up,
    Right,
    Down,
};

constexpr Action actions[5] { Action::None, Action::Left, Action::Up, Action::Right, Action::Down };
constexpr int n_cells = 5;
constexpr int n_actions = 4;
std::ostream& operator<<(std::ostream& out, const Action action);
std::ostream& operator<<(std::ostream& out, const Cell cell);

struct State {
    int x;
    int y;
};

class Game {
public:
    Game() = default;

    /// Initial input containing the game's parameters
    void init_input(std::istream& _in)
    {
        _in >> m_height >> m_width >> going_back_limit;
        m_buf.reserve(m_width);
        m_grid.reserve(m_width * m_height);
        std::fill_n(std::back_inserter(m_grid), m_width * m_height, Cell::Unknown);
    }

    /// Update the grid if we see new cells
    void turn_input(std::istream& _in)
    {
        _in >> m_state.y >> m_state.x;
        _in.ignore();
        for (int y = 0; y < m_height; ++y) {
            std::getline(_in, m_buf);
            for (int x = 0; x < m_width; ++x) {
                char c = m_buf[x];
                switch (c) {
                case '?':
                    break;
                case '#':
                    cell_at(x, y) = Cell::Wall;
                    break;
                case '.':
                    cell_at(x, y) = Cell::Empty;
                    break;
                case 'T':
                    cell_at(x, y) = Cell::Start;
                    break;
                case 'C':
                    cell_at(x, y) = Cell::Target;
                    break;
                default: {
                    std::cerr << "Game::turn_input: unknown character read"
                              << std::endl;
                    assert(false);
                }
                }
            }
        }
    }

    void show(std::ostream& out)
    {
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x)
                out << cell_at(x, y);
            out << '\n';
        }
        out << std::endl;
    }

    Cell& cell_at(int x, int y)
    {
        return m_grid[x + y * m_width];
    }

    Cell cell_at(int x, int y) const
    {
        return m_grid[x + y * m_width];
    }

    const std::vector<Cell> grid() const { return m_grid; }

    const State& state() const { return m_state; }
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    std::vector<Cell> m_grid;
    std::string m_buf;
    State m_state;
    int m_width;
    int m_height;
    int going_back_limit;
};

class Agent {
public:
    Agent(Game& _game)
        : game(_game)
    {
    }

    bool is_unsafe(int x, int y)
    {
        Cell cell = game.cell_at(x, y);
        return cell == Cell::Unknown || cell == Cell::Wall;
    }

    template <Action action>
    bool is_safe(const State& state)
    {
        bool ret = true;
        if constexpr (action == Action::Left) {
            ret &= state.x > 0;
            ret &= !is_unsafe(state.x - 1, state.y);
        } else if constexpr (action == Action::Up) {
            ret &= state.y > 0;
            ret &= !is_unsafe(state.x, state.y - 1);
        } else if constexpr (action == Action::Right) {
            ret &= state.x < game.width() - 1;
            ret &= !is_unsafe(state.x + 1, state.y);
        } else if constexpr (action == Action::Down) {
            ret &= state.x < game.height() - 1;
            ret &= !is_unsafe(state.x, state.y + 1);
        } else {
            std::cerr << "Agent::is_safe: unknown action"
                      << std::endl;
            assert(false);
        }

        return ret;
    }

    Action best_action()
    {
        m_buf.clear();
        auto inserter = std::back_inserter(m_buf);
        const State& state = game.state();

        if (is_safe<actions[1]>(game.state())) {
            inserter = actions[1];
        }
        if (is_safe<actions[2]>(game.state())) {
            inserter = actions[2];
        }
        if (is_safe<actions[3]>(game.state())) {
            inserter = actions[3];
        }
        if (is_safe<actions[4]>(game.state())) {
            inserter = actions[4];
        }

        if (m_buf.empty()) {
            std::cerr << "Agent::best_action: no actions available"
                      << std::endl;
            assert(false);
        }

        return m_buf[rand() % m_buf.size()];
    }

private:
    const Game& game;
    std::vector<Action> m_buf;
    std::deque<Action> best_actions;
};


std::ostream& operator<<(std::ostream& out, const Cell cell)
{
    switch (cell) {
    case Cell::Unknown:
        return out << '?';
    case Cell::Wall:
        return out << '#';
    case Cell::Empty:
        return out << '.';
    case Cell::Start:
        return out << 'T';
    case Cell::Target:
        return out << 'C';
    default: {
        std::cerr << "Game::show: unknown cell found"
                  << std::endl;
        assert(false);
    }
    }
}

std::ostream& operator<<(std::ostream& out, const Action action)
{
    switch (action) {
    case Action::Left:
        return out << "LEFT";
    case Action::Up:
        return out << "UP";
    case Action::Right:
        return out << "RIGHT";
    case Action::Down:
        return out << "DOWN";
    case Action::None:
    default: {
        std::cerr << "trying to output Action::None"
                  << std::endl;
        assert(false);
    }
    }
}

int main(int argc, char* argv[])
{
    Game game;
    game.init_input(std::cin);
    game.turn_input(std::cin);

    Agent agent(game);

    game.show(std::cerr);

    while (true) {
        Action action = agent.best_action();
        std::cout << action << std::endl;
        game.turn_input(std::cin);
    }

    return 0;
}
