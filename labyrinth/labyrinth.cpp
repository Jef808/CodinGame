#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <iostream>
#include <set>
#include <string>
#include <tuple>
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
constexpr int n_actions = 4;

std::ostream& operator<<(std::ostream& out, const Action action);
std::ostream& operator<<(std::ostream& out, const Cell cell);
std::ostream& operator<<(std::ostream& out, const Point point);

/// To insert into a std::set
bool operator<(const Point& a, const Point& b) {
    return a.y < b.y || (a.y == b.y && a.x < b.x);
}
bool operator==(const Point& a, const Point& b) {
    return a.x == b.x && a.y == b.y;
}
Point operator+(const Point& a, const Point& b) {
    return { a.x + b.x, a.y + b.y };
}
Point& operator+=(Point& p, const Point& dp) {
    p = p + dp;
    return p;
}

class Game {
public:

    Game() = default;

    /// Initialize the grid and the game's parameters
    void init_input(std::istream& _in)
    {
        _in >> m_height >> m_width >> going_back_alarm_time;
        m_buf.reserve(m_width);
        m_grid.reserve((m_width + 2) * (m_height + 2));
        std::fill_n(std::back_inserter(m_grid), (m_width + 2) * (m_height + 2), Cell::Unknown);
        m_target_found = false;
        m_target.x = -1;
        m_target.y = -1;

        // Add a layer of walls around the grid to make checking for out of bound positions
        // unnecessary
        for (int x = 0; x < m_width + 2; ++x) {
            m_grid[x] = m_grid[x + (m_height + 2 - 1) * (m_width + 2)] = Cell::Wall;
        }
        for (int y = 1; y < m_height + 1; ++y) {
            m_grid[y * (m_width + 2)] = m_grid[(y + 1) * (m_width + 2) - 1] = Cell::Wall;
        }
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
                if (cell_at(x, y) != Cell::Unknown)
                    continue;
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
    int index_of(const Point& p) const {
        return p.x + p.y * m_width;
    }
    Cell cell_at(int x, int y) const
    {
        return m_grid[(x + 1) + (y + 1) * (m_width + 2)];
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
    const Point& state() const {
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
    int going_back_alarm_time;
    Point m_state;
    bool m_target_found;
    Point m_start;
    Point m_target;

    Cell& cell_at(int x, int y)
    {
        return m_grid[(x + 1) + (y + 1) * (m_width + 2)];
    }
};

class Agent {
public:
    Agent(Game& _game)
        : game(_game)
    {
        reached_target = false;
        max_distance = game.width() * game.height();
        gradient.reserve(game.width() * game.height());
        fog_boundary.reserve(4 * (game.width() + game.height()));
    }

    Action best_action()
    {
        Action action;
        bool found_path = false;

        if (game.target_found()) {
            reached_target |= game.state() == game.target();
            if (!reached_target) {
                std::cerr << "Going towards target" << std::endl;
                std::tie(action, found_path) = go_towards(game.target());
            }
            else {
                std::cerr << "Going towards start" << std::endl;
                std::tie(action, found_path) = go_towards(game.start());
            }
        }

        if (!found_path) {
            std::cerr << "Going towards nearest fog" << std::endl;
            action = towards_nearest_fog();
        }

        return action;
    }

private:
    const Game& game;
    std::vector<Point> fog_boundary;
    std::vector<int> gradient;
    Point offsets[4] { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };
    int max_distance;
    bool reached_target;

    /// Mark the distances from the source to each discovered and fog-boundary points
    void compute_gradient(const Point& source)
    {
        gradient.clear();
        fog_boundary.clear();

        std::fill_n(std::back_inserter(gradient), game.width() * game.height(), max_distance);

        std::set<Point> seen({source});
        std::deque<Point> queue({source});

        gradient[game.index_of(source)] = 0;

        while (!queue.empty()) {

            Point point = queue.front();
            int distance = gradient[game.index_of(point)];
            queue.pop_front();

            for (const auto& dp : offsets) {

                auto [nbh, not_seen] = seen.insert(point + dp);

                if (!not_seen || is_wall(*nbh))
                    continue;

                gradient[game.index_of(*nbh)] = distance + 1;

                if (is_fog(*nbh))
                    fog_boundary.push_back(*nbh);
                else
                    queue.push_back(*nbh);
            }
        }
    }

    /// Get the direction to the nearest fog square
    Action towards_nearest_fog()
    {
        compute_gradient(game.state());

        Point target = *std::min_element(fog_boundary.begin(), fog_boundary.end(), [&](const auto& a, const auto& b){
            return gradient[game.index_of(a)] < gradient[game.index_of(b)];
        });

        auto [action, found] = go_towards(target);

        if (!found) {
            std::cerr << "Agent::to_nearest_fog: couldn't find a path"
                << std::endl;
            assert(false);
        } else {
            return action;
        }
    }

    std::pair<Action, bool> go_towards(const Point& target)
    {
        compute_gradient(target);
        Action action = shortest_path(game.state(), target);

        return { action, action != Action::None };
    }

    Action shortest_path(const Point& start, const Point& end)
    {
        int distance = gradient[game.index_of(start)];
        Point p = start;

        Action first_action = Action::None;

        while (distance > 0) {

            bool found_closer_point = false;

            for (int j = 0; j < 4; ++j) {

                Point cand = p + offsets[j];

                // check distance > 1 because we could be targeting a fog point
                if (distance > 1 && is_fog(cand))
                    continue;

                int cand_dist = gradient[game.index_of(cand)];

                // If found direction reducing distance
                if (cand_dist == distance - 1) {

                    found_closer_point = true;

                    // Record the first action
                    if (first_action == Action::None)
                        first_action = actions[j + 1];

                    p = cand;
                    distance = cand_dist;
                    break;
                }
            }

            // If no candidates were found with shorter distance, fail
            if (!found_closer_point)
                return Action::None;
        }

        return first_action;
    }

    bool is_wall(const Point& point)
    {
        return game.cell_at(point.x, point.y) == Cell::Wall;
    }

    bool is_fog(const Point& point)
    {
        return game.cell_at(point.x, point.y) == Cell::Unknown;
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

    while (true) {
        Action action = agent.best_action();
        std::cout << action << std::endl;
        game.turn_input(std::cin);
    }

    return 0;
}
