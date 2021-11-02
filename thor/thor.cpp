#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

constexpr int width = 40;
constexpr int height = 18;
constexpr int max_turns = 200;

enum class Cell {
    Empty, Giant, Thor, Boundary
};

struct Point {
    int x;
    int y;
};

enum class Action {
    None = 0, N = 1, NE = 2, E = 3, SE = 4 , S = 5, SW = 6, W = 7, NW = 8, Wait, Strike
};

constexpr int NbActions = 10;
constexpr int NbDirections = 8;

constexpr Point offsets[NbDirections] {
    {0, 1}, {1, 1}, {1, 0}, {1, -1},
    {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}
};

constexpr Point direction_of(const Action action) {
    return offsets[static_cast<int>(action) + 1];
}

Point operator+(const Point& point, const Point& dp) {
    return { point.x + dp.x, point.y + dp.y };
}
Point& operator+=(Point& point, const Point& dp) {
    point = point + dp;
    return point;
}

struct State {
    Point pos;
    int n_strikes;
    int n_giants;
};

std::ostream& operator<<(std::ostream& out, const Cell cell);
std::ostream& operator<<(std::ostream& out, const Action action);

class Game {
public:

    Game() = default;

    void init_input(std::istream& _in) {
        _in >> m_state.pos.x >> m_state.pos.y;
        m_giants.reserve(100);
    }

    void turn_input(std::istream& _in) {
        reset();
        _in >> m_state.n_strikes >> m_state.n_giants;
        for (int i = 0; i < m_state.n_giants; ++i) {
            int gx, gy;
            _in >> gx >> gy;
            cell_at(gx, gy) = Cell::Giant;
            m_giants.push_back({gx, gy});
        }
    }

    void reset() {
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

    void update_pos(Action action) {
        if (action == Action::None) {
            std::cerr << "Game::update_pos: received Action::None"
                << std::endl;
            assert(false);
        }
        if (action == Action::Wait || action == Action::Strike)
            return;
        m_state.pos += direction_of(action);
    }

    void show(std::ostream& out) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                out << cell_at(x, y);
            }
            out << '\n';
        }
        out << std::endl;
    }

    const State& state() const {
        return m_state;
    }
    const Cell& cell_at(int x, int y) const {
        return m_grid[(x + 1) + (y + 1) * (width + 2)];
    }
    const Cell& cell_at(const Point& point) {
        return m_grid[(point.x + 1) + (point.y + 1) * (width + 2)];
    }
    int index_of(const Point& point) const {
        return point.x + point.y * width;
    }

private:
    std::array<Cell, (width + 2) * (height + 2)> m_grid;
    std::vector<Point> m_giants;
    State m_state;

    Cell& cell_at(int x, int y) {
        return m_grid[(x + 1) + (y + 1) * (width + 2)];
    }
};

class Agent {
public:
    Agent(const Game& _game)
        : game(_game)
    {
    }

    Action best_action() {
        return Action::Wait;
    }

private:
    const Game& game;
};

std::ostream& operator<<(std::ostream& out, const Cell cell) {
    switch(cell) {
        case Cell::Empty: return out << '.';
        case Cell::Giant: return out << 'G';
        case Cell::Thor:  return out << 'T';
        case Cell::Boundary: return out << '#';
        default: {
            std::cerr << "invalid cell sent to output stream"
                      << std::endl;
            assert(false);
        }
    }
}

std::ostream& operator<<(std::ostream& out, const Action action) {
    switch(action) {
        case Action::Wait: return out << "WAIT";
        case Action::Strike: return out << "STRIKE";
        case Action::N: return out << 'N';
        case Action::NE: return out << "NE";
        case Action::E: return out << 'E';
        case Action::SE: return out << "SE";
        case Action::S: return out << 'S';
        case Action::SW: return out << "SW";
        case Action::W: return out << 'W';
        case Action::NW: return out << "NW";
        default: {
            std::cerr << "invalid action sent to output stream"
                      << std::endl;
            assert(false);
        }
    }
}

int main(int argc, char *argv[]) {

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
    }

    return 0;
}
