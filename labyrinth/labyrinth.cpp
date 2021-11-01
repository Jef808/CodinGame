#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <iostream>
#include <set>
#include <string>
#include <utility>
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

struct Point {
    int x;
    int y;
};

constexpr Action actions[5] { Action::None, Action::Left, Action::Up, Action::Right, Action::Down };
constexpr int n_cells = 5;
constexpr int n_actions = 4;
std::ostream& operator<<(std::ostream& out, const Action action);
std::ostream& operator<<(std::ostream& out, const Cell cell);
std::ostream& operator<<(std::ostream& out, const Point point);

/// To insert into a std::set
bool operator<(const Point& a, const Point& b) {
    return a.y > b.y || a.x < b.x;
}

typedef Point State;

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
        m_target_found = false;
        m_target.x = -1;
        m_target.y = -1;
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
                    m_start.x = x;
                    m_start.y = y;
                    break;
                case 'C':
                    cell_at(x, y) = Cell::Target;
                    m_target_found = true;
                    m_target.x = x;
                    m_target.y = y;
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

    int width() const {
        return m_width;
    }
    int height() const {
        return m_height;
    }
    const std::vector<Cell> grid() const {
        return m_grid;
    }
    const State& state() const {
        return m_state;
    }
    bool target_found() const {
        return m_target_found;
    }
    const Point& target() const {
        return m_target;
    }
    const Point& start() const {
        return m_start;
    }

private:
    std::vector<Cell> m_grid;
    std::string m_buf;
    int m_width;
    int m_height;
    int going_back_limit;
    State m_state;
    bool m_target_found;
    Point m_start;
    Point m_target;
};

class Agent {
public:
    Agent(Game& _game)
        : game(_game)
    {
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
    std::vector<Point> fog_boundary;
    std::vector<Point> fog_buffer;

    void shortest_path(int x_start, int y_start, int x_target, int y_target)
    {

    }

    void update_fog_boundary()
    {
        if (fog_boundary.empty())
            return;

        std::set<Point> seen;
        fog_buffer.clear();
        auto fog_inserter = std::back_inserter(fog_buffer);

        for (const Point& current : fog_boundary) {

            /// Mark as fog and don't look at neighbours if cell is still fog
            if (game.cell_at(current.x, current.y) == Cell::Unknown) {
                auto [it, not_seen] = seen.insert({current.x, current.y});
                if (not_seen)
                    fog_inserter = current;
                continue;
            }

            /// Look at four neighbours for newly discovered cells
            for (auto [dx, dy] : { std::make_pair(-1, 0), std::make_pair(0, -1),
                    std::make_pair(1, 0), std::make_pair(0, 1)} ) {
                if (game.cell_at(current.x + dx, current.y + dy) == Cell::Unknown) {
                    auto [it, not_seen] = seen.insert({ current.x, current.y });
                    if (not_seen)
                        fog_inserter = { current.x + dx, current.y + dy };
                }
            }
        }

        std::swap(fog_boundary, fog_buffer);
    }

    /// Add the outside cells of a 5x5 box centered at starting position
    void init_fog_boundary()
    {
        fog_boundary.reserve(4 * game.width() * game.height());
        fog_buffer.reserve(4 * game.width() * game.height());

        int startx = game.start().x, starty = game.start().y;

        /// Top and bottom sides
        for (int y : { starty - 3, starty + 3 }) {
            if (y < 0 || y >= game.height())
                continue;
            for (int x = std::max(startx - 3, 0); x < std::min(startx + 4, game.width()); ++x) {
                    continue;
                fog_boundary.push_back({ x, y });
            }

        }

        /// Left and Right sides (avoiding the corners)
        for (int x : { startx - 3, startx + 3}) {
            if (x < 0 || x >= game.width())
                continue;
            for (int y = std::max(starty - 2, 0); y < std::min(starty + 3, game.width()); ++y)
                fog_boundary.push_back({ x, y });
        }
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

std::ostream& operator<<(std::ostream& out, const Point point)
{
    return out << '('  << point.x
               << ", " << point.y << ')';
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
