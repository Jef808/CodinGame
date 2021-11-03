#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <iostream>
#include <numeric>
#include <set>
#include <string>
#include <vector>

constexpr int width = 40;
constexpr int height = 18;
constexpr int max_turns = 200;
constexpr double PI = 3.14159265;

enum class Cell {
    Empty,
    Giant,
    Thor,
    Boundary
};

struct Point {
    int x;
    int y;
};

enum class Action {
    None = 0,
    N = 1,
    NE = 2,
    E = 3,
    SE = 4,
    S = 5,
    SW = 6,
    W = 7,
    NW = 8,
    Wait = 9,
    Strike = 10
};

constexpr int NbActions = 10;
constexpr int NbDirections = 8;

constexpr Action actions[NbActions] {
    Action::N, Action::NE, Action::E, Action::SE, Action::S,
    Action::SW, Action::W, Action::NW, Action::Wait, Action::Strike
};

constexpr Point offsets[NbDirections] {
    { 0, -1 }, // North
    { 1, -1 }, // North East
    { 1, 0 }, // East
    { 1, 1 }, // South East
    { 0, 1 }, // South
    { -1, 1 }, // South West
    { -1, 0 }, // West
    { -1, -1 } // North West
};

std::ostream& operator<<(std::ostream& out, const Cell cell);
std::ostream& operator<<(std::ostream& out, const Action action);
std::ostream& operator<<(std::ostream& out, const Point& point);

constexpr Point direction_of(const Action action)
{
    assert(action != Action::None);
    if (action == Action::Wait || action == Action::Strike)
        return { 0, 0 };
    return offsets[static_cast<int>(action) - 1];
}

Point operator+(const Point& point, const Point& dp)
{
    return { point.x + dp.x, point.y + dp.y };
}
Point operator*(const Point& point, int s)
{
    return { point.x * s, point.y * s };
}
Point& operator*=(Point& point, int s)
{
    point = point * s;
    return point;
}
Point& operator+=(Point& point, const Point& dp)
{
    point = point + dp;
    return point;
}

int lattice_distance(const Point& a, const Point& b)
{
    return std::max(abs(a.x - b.x), abs(a.y - b.y));
}

struct State {
    Point pos;
    int n_strikes;
    int n_giants;
};

class Game {
public:
    using point_iterator = std::vector<Point>::const_iterator;
    using point_range = std::pair<point_iterator, point_iterator>;

    Game() = default;

    void init_input(std::istream& _in)
    {
        _in >> m_state.pos.x >> m_state.pos.y;
        m_giants.reserve(100);
    }

    void turn_input(std::istream& _in)
    {
        reset();
        _in >> m_state.n_strikes >> m_state.n_giants;
        for (int i = 0; i < m_state.n_giants; ++i) {
            int gx, gy;
            _in >> gx >> gy;
            cell_at(gx, gy) = Cell::Giant;
            m_giants.push_back({ gx, gy });
        }
    }

    void reset()
    {
        m_giants.clear();
        m_grid.fill(Cell::Empty);
        for (int y = 0; y < height + 2; ++y) {
            m_grid[y * (width + 2)] = m_grid[(y + 1) * (width + 2) - 1] = Cell::Boundary;
        }
        for (int x = 1; x < width + 1; ++x) {
            m_grid[x] = m_grid[x + (height + 2 - 1) * (width + 2)] = Cell::Boundary;
        }
        cell_at(m_state.pos.x, m_state.pos.y) = Cell::Thor;
    }

    void update_pos(Action action)
    {
        if (action == Action::None) {
            std::cerr << "Game::update_pos: received Action::None"
                      << std::endl;
            assert(false);
        }
        if (action == Action::Wait || action == Action::Strike)
            return;
        m_state.pos += direction_of(action);
    }

    void show(std::ostream& out)
    {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                out << cell_at(x, y);
            }
            out << '\n';
        }
        out << '\n';
        out << "Thor pos: ("
            << m_state.pos.x << ", "
            << m_state.pos.y << ')';
        out << std::endl;
    }

    const State& state() const
    {
        return m_state;
    }
    point_range giants() const
    {
        return std::make_pair(m_giants.begin(), m_giants.end());
    }
    const Cell& cell_at(int x, int y) const
    {
        return m_grid[(x + 1) + (y + 1) * (width + 2)];
    }
    const Cell& cell_at(const Point& point) const
    {
        return m_grid[(point.x + 1) + (point.y + 1) * (width + 2)];
    }
    int index_of(const Point& point) const
    {
        return point.x + point.y * width;
    }
    bool is_boundary(const Point& point) const
    {
        return point.x < 0 || point.x > width - 1 || point.y < 0 || point.y > height - 1;
    }
    int dist_to_boundary(const Point& point) const
    {
        return std::min({ point.x, point.y, width - 1 - point.x, height - 1 - point.y });
    }

private:
    std::array<Cell, (width + 2) * (height + 2)> m_grid;
    std::vector<Point> m_giants;
    State m_state;

    Cell& cell_at(int x, int y)
    {
        return m_grid[(x + 1) + (y + 1) * (width + 2)];
    }
};

class Agent {

    struct ExtPoint {
        explicit ExtPoint(const Point& p)
            : pos { p }
            , distance { -1 }
        {
        }
        ExtPoint(const Point& p, int dist)
            : pos { p }
            , distance { dist }
        {
        }
        Point pos;
        int distance;
    };

public:
    Agent(const Game& _game)
        : game(_game)
    {
        gradient.reserve(width * height);
    }

    Action best_action()
    {
        compute_distances(game.state().pos);

        int n_strikes = game.state().n_strikes;
        int n_giants = game.state().n_giants;

        if (result_of_strike() >= n_giants)
            return Action::Strike;

        int min_kill = 1 + n_giants / n_strikes;

        if (result_of_strike() >= 1.5 * (n_giants / n_strikes))
            return Action::Strike;

        int closest = closest_giant();
        double density1 = ball_density(closest * 1.2);
        double density2 = ball_density(closest * 1.5);
        double density3 = ball_density(closest * 2.0);

        double avg_pw_dist = average_pairwise_dist();

        if (closest_giant() < 2) {
            auto [action, dist] = escape();
            if (dist > 1) {
                std::cerr << "Escaping because closest < 2" << std::endl;
                return action;
            }
        }

        //auto [x_diam, y_diam] = side_length();
        auto step_size = closest / 4; //std::max(width / x_diam, height / y_diam);

        std::cerr << "Closest: " << closest
                  << "Density of ball(closets * 1.2): "
                  << density1 << '\n'
                  << "Density of ball(closest * 1.5): "
                  << density2 << '\n'
                  << "Step size: "
                  << step_size << std::endl;

        if (n_giants == giants_in_ball(game.state().pos, closest * 2)) {
            std::cerr << "Going in because densities are equal... the end?" << std::endl;
            return go_in(step_size);
        }

        if (density1 < density2) {
            std::cerr << "Going in because inner density is smaller than outer" << std::endl;
            return go_in(step_size);
        } else {
            std::cerr << "Escaping because outer density is smaller than inner" << std::endl;
            auto [action, dist] = escape();
            if (dist > 1)
                return action;
        }

        return Action::Strike;
    }

private:
    const Game& game;
    std::vector<Action> action_buf;
    std::vector<int> gradient;
    std::vector<ExtPoint> giants;
    const int max_distance = width + height;

    int closest_giant()
    {
        return std::min_element(giants.begin(), giants.end(), [](const auto& a, const auto& b) {
            return a.distance < b.distance;
        })->distance;
    }

    int sum_of_dist()
    {
        return std::accumulate(giants.begin(), giants.end(), 0, [](auto& s, const auto& g) {
            return s += g.distance;
        });
    }

    double average_pairwise_dist()
    {
        double ret;
        for (const auto& g : giants) {
            int avg = 0;
            for (const auto& gg : giants) {
                avg += lattice_distance(g.pos, gg.pos);
            }
            ret += avg / giants.size();
        }
        return ret / giants.size();
    }

    std::pair<Action, double> escape()
    {
        Action best_action = Action::Wait;
        int closest = closest_giant();

        double largest_distance = closest;

        for (int i = 0; i < 8; ++i) {
            Action action = actions[i];
            Point p = result_of(action);
            if (game.is_boundary(p))
                continue;
            compute_distances(p);
            closest = closest_giant();
            auto [x, y] = side_length();
            int dist = closest;
            if (dist > largest_distance) {
                largest_distance = dist;
                best_action = action;
            }
        }

        return { best_action, largest_distance };
    }

    Action go_in(int step)
    {
        Action best_action = Action::Wait;
        int smallest_sum = sum_of_dist();
        auto [smallest_x, smallest_y] = side_length();
        int best_score = smallest_sum;
        int dist_bdry = game.dist_to_boundary(game.state().pos);
        for (int i = 0; i < 8; ++i) {
            Action action = actions[i];
            Point p = result_of(action);
            compute_distances(p);
            if (closest_giant() < 2)
                continue;
            for (int s = 1; s < 5; ++s) {
                p *= step / 5;
                if (game.is_boundary(p))
                    break;
                int sum = sum_of_dist() + game.state().n_giants * (game.dist_to_boundary(p) > dist_bdry);
                auto [x, y] = side_length();
                if (sum < smallest_sum) {
                    int score = sum;
                    if (x * y < smallest_x * smallest_y) {
                        score += x * y - smallest_x * smallest_y;
                    }
                    if (score < best_score) {
                        smallest_sum = sum;
                        best_score = score;
                        best_action = action;
                    }
                }
            }
        }

        return best_action;
    }

    std::pair<int, int> side_length()
    {
        int max_x = 0;
        int max_y = 0;
        for (const auto& g : giants) {
            for (const auto& gg : giants) {
                int x = abs(g.pos.x - gg.pos.x);
                int y = abs(g.pos.y - gg.pos.y);
                if (x > max_x)
                    max_x = x;
                if (y > max_y)
                    max_y = y;
            }
        }
        return { max_x, max_y };
    }

    double ball_density(int radius)
    {
        double n_giants = std::count_if(giants.begin(), giants.end(), [r = radius](const auto& a) {
            return a.distance < r;
        });
        return n_giants / radius;
    }

    int giants_in_ball(const Point& point, int radius)
    {
        return std::count_if(giants.begin(), giants.end(), [r = radius](const auto& a) {
            return a.distance < r;
        });
    }

    int result_of_strike()
    {
        return std::count_if(giants.begin(), giants.end(), [&](const auto& giant) {
            return giant.distance < 5;
        });
    }

    Point result_of(Action action) const
    {
        if (action == Action::None) {
            std::cerr << "Agent::result_of: received Action::None"
                      << std::endl;
            assert(false);
        }
        if (action == Action::Wait || action == Action::Strike)
            return game.state().pos;

        return game.state().pos + direction_of(action);
    }

    /// Starting at Thor's position, compute the manhattan distance to each square in the giant range
    void compute_distances(const Point& center)
    {
        giants.clear();
        auto [beg, end] = game.giants();
        for (auto it = beg; it != end; ++it) {
            giants.push_back({ { it->x, it->y }, lattice_distance(center, *it) });
        }
    }
};

std::ostream& operator<<(std::ostream& out, const Point& point)
{
    return out << '(' << point.x
               << ", " << point.y << ')';
}

std::ostream& operator<<(std::ostream& out, const Cell cell)
{
    switch (cell) {
    case Cell::Empty:
        return out << '.';
    case Cell::Giant:
        return out << 'G';
    case Cell::Thor:
        return out << 'T';
    case Cell::Boundary:
        return out << '#';
    default: {
        std::cerr << "invalid cell sent to output stream"
                  << std::endl;
        assert(false);
    }
    }
}

std::ostream& operator<<(std::ostream& out, const Action action)
{
    switch (action) {
    case Action::Wait:
        return out << "WAIT";
    case Action::Strike:
        return out << "STRIKE";
    case Action::N:
        return out << 'N';
    case Action::NE:
        return out << "NE";
    case Action::E:
        return out << 'E';
    case Action::SE:
        return out << "SE";
    case Action::S:
        return out << 'S';
    case Action::SW:
        return out << "SW";
    case Action::W:
        return out << 'W';
    case Action::NW:
        return out << "NW";
    default: {
        std::cerr << "invalid action sent to output stream"
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

    game.show(std::cerr);

    Agent agent(game);

    while (true) {
        Action action = agent.best_action();
        std::cout << action << std::endl;
        game.update_pos(action);
        game.turn_input(std::cin);
        game.show(std::cerr);
    }

    return 0;
}
