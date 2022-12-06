#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <iterator>
#include <set>
#include <tuple>
#include <vector>

enum class Direction { SOUTH, EAST, NORTH, WEST };

std::ostream& operator<<(std::ostream& out, Direction dir) {
    switch (dir) {
    case Direction::SOUTH:
        return out << "SOUTH";
    case Direction::EAST:
        return out << "EAST";
    case Direction::NORTH:
        return out << "NORTH";
    case Direction::WEST:
        return out << "WEST";
    }
}

enum class Tile {
    BLANK,
    START,
    END,
    BREAKABLE,
    UNBREAKABLE,
    SOUTH,
    EAST,
    NORTH,
    WEST,
    INVERTER,
    BEER,
    TELEPORTER
};

Tile char2tile(char c) {
    switch (c) {
    case ' ':
        return Tile::BLANK;
    case '@':
        return Tile::START;
    case '$':
        return Tile::END;
    case 'X':
        return Tile::BREAKABLE;
    case '#':
        return Tile::UNBREAKABLE;
    case 'S':
        return Tile::SOUTH;
    case 'E':
        return Tile::EAST;
    case 'N':
        return Tile::NORTH;
    case 'W':
        return Tile::WEST;
    case 'I':
        return Tile::INVERTER;
    case 'B':
        return Tile::BEER;
    case 'T':
        return Tile::TELEPORTER;
    default:
        std::cerr << "char2tile: Invalid tile character";
        throw;
    }
}

char tile2char(Tile t) {
    switch (t) {
    case Tile::BLANK:
        return ' ';
    case Tile::START:
        return '@';
    case Tile::END:
        return '$';
    case Tile::BREAKABLE:
        return 'X';
    case Tile::UNBREAKABLE:
        return '#';
    case Tile::SOUTH:
        return 'S';
    case Tile::EAST:
        return 'E';
    case Tile::NORTH:
        return 'N';
    case Tile::WEST:
        return 'W';
    case Tile::INVERTER:
        return 'I';
    case Tile::BEER:
        return 'B';
    case Tile::TELEPORTER:
        return 'T';
    }
}

struct StateDescriptor {
    size_t idx{0};
    Direction dir{Direction::SOUTH};
    bool breaker{false};
    std::set<size_t> broken;
    bool inverted_priority{false};

    bool operator==(const StateDescriptor& other) const {
        return idx == other.idx && dir == other.dir && breaker == other.breaker
               && broken == other.broken
               && inverted_priority == other.inverted_priority;
    }
};

class State {
  public:
    State() = default;

    friend State init(std::istream& is);

    /**
     * Step towards the next position and return the direction taken.
     */
    Direction step();

    [[nodiscard]] Direction direction() const { return m_desc.dir; }

    [[nodiscard]] size_t idx() const { return m_desc.idx; }

    [[nodiscard]] bool is_okay() const {
        return !(is_in_loop() || is_at_end());
    }

    [[nodiscard]] bool is_in_loop() const {
        return std::find(m_seen.begin(), m_seen.end(), m_desc) != m_seen.end();
        // return m_seen.count(m_desc);
    }

    [[nodiscard]] bool is_at_end() const {
        return m_grid[m_desc.idx] == Tile::END;
    }

    void view(std::ostream& out) const;

  private:
    size_t m_width{0};
    size_t m_height{0};
    std::vector<Tile> m_grid;
    StateDescriptor m_desc;
    std::vector<StateDescriptor> m_seen;
    // std::set<StateDescriptor> m_seen;

    [[nodiscard]] const std::array<Direction, 4>& priorities() const;
    [[nodiscard]] bool is_blocked(Direction dir) const;
    [[nodiscard]] int advance(Direction dir) const noexcept;
    void apply_current_tile_side_effects();
    void apply_turn_if_next_tile_blocked();
    bool check_if_looping();
    void register_current_state();
};

State init(std::istream& is) {
    State state;
    is >> state.m_height >> state.m_width;
    is.ignore();
    state.m_grid.reserve(
            static_cast<std::vector<Tile>::size_type>(state.m_height)
            * state.m_width);

    std::string buf;
    for (int i = 0; i < state.m_height; ++i) {
        std::getline(is, buf);
        for (auto c : buf) {
            Tile tile = state.m_grid.emplace_back(char2tile(c));
            if (tile == Tile::START) state.m_desc.idx = state.m_grid.size() - 1;
        }
    }

    return state;
}

int State::advance(Direction dir) const noexcept {
    switch (dir) {
    case Direction::SOUTH:
        return static_cast<int>(m_desc.idx) + static_cast<int>(m_width);
    case Direction::EAST:
        return static_cast<int>(m_desc.idx) + 1;
    case Direction::WEST:
        return static_cast<int>(m_desc.idx) - 1;
    case Direction::NORTH:
        return static_cast<int>(m_desc.idx) - static_cast<int>(m_width);
    }
}

const std::array<Direction, 4>& State::priorities() const {
    static constexpr std::array<Direction, 4> directions{
            Direction::SOUTH, Direction::EAST, Direction::NORTH,
            Direction::WEST};
    static constexpr std::array<Direction, 4> reversed_directions{
            Direction::WEST, Direction::NORTH, Direction::EAST,
            Direction::SOUTH};
    return m_desc.inverted_priority ? reversed_directions : directions;
}

bool State::is_blocked(Direction dir) const {
    const int nidx = advance(dir);
    assert(0 <= nidx && nidx < m_grid.size() // NOLINT
           && "index out of bounds");

    switch (m_grid[nidx]) {
    case Tile::UNBREAKABLE:
        return true;
    case Tile::BREAKABLE:
        return !(m_desc.breaker || m_desc.broken.count(nidx));
    default:
        return false;
    }
}

void State::apply_current_tile_side_effects() {
    switch (m_grid[m_desc.idx]) {
    case Tile::SOUTH:
        m_desc.dir = Direction::SOUTH;
        break;
    case Tile::EAST:
        m_desc.dir = Direction::EAST;
        break;
    case Tile::NORTH:
        m_desc.dir = Direction::NORTH;
        break;
    case Tile::WEST:
        m_desc.dir = Direction::WEST;
        break;
    case Tile::BEER:
        m_desc.breaker = !m_desc.breaker;
        break;
    case Tile::INVERTER:
        m_desc.inverted_priority = !m_desc.inverted_priority;
        break;
    case Tile::BREAKABLE:
        m_desc.broken.insert(m_desc.idx);
        break;
    case Tile::TELEPORTER: {
        const auto next_teleporter = [&](size_t idx = 0) {
            return std::distance(m_grid.begin(),
                                 std::find(m_grid.begin() + int(idx) + 1,
                                           m_grid.end(), Tile::TELEPORTER));
        };
        const size_t first_teleporter = next_teleporter(0);
        m_desc.idx = first_teleporter == m_desc.idx
                             ? next_teleporter(m_desc.idx)
                             : first_teleporter;
        break;
    }
    default:
        break;
    }
}

inline void State::apply_turn_if_next_tile_blocked() {
    if (is_blocked(m_desc.dir))
        m_desc.dir =
                *std::find_if(priorities().begin(), priorities().end(),
                              [&](Direction dir) { return !is_blocked(dir); });
}

inline void State::register_current_state() {
    m_seen.push_back(m_desc);
    // m_seen.insert(m_desc);
}

Direction State::step() {
    register_current_state();

    apply_turn_if_next_tile_blocked();
    const Direction direction_taken = m_desc.dir;

    m_desc.idx = advance(m_desc.dir);
    apply_current_tile_side_effects();
    return direction_taken;
}

void State::view(std::ostream& out) const {
    for (int i = 0; i < m_height; ++i) {
        for (int j = 0; j < m_width; ++j) {
            const size_t idx = i * m_width + j;
            char c = tile2char(m_grid[idx]);
            if (idx == m_desc.idx) c = '*';
            else if (m_desc.broken.count(idx))
                c = 'x';
            out << c << ' ';
        }
        out << '\n';
    }
    out << "(" << m_desc.idx % m_width << ", " << m_desc.idx / m_height << ")";
    if (is_at_end()) out << " -> " << m_desc.dir << '\n';
    if (m_desc.breaker) out << "BREAKER" << '\n';
    out << std::endl;
}

int main() {
    using std::cerr;
    using std::cin;
    using std::cout;
    using std::endl;

    State state = init(cin);

    std::vector<Direction> directions;
    state.view(cerr);

    cerr << "\nComputing path..." << endl;

    while (state.is_okay()) {
        directions.push_back(state.step());
        state.view(cerr);
    }

    if (state.is_in_loop()) {
        cout << "LOOP" << endl;
    } else {
        std::copy(directions.begin(), directions.end(),
                  std::ostream_iterator<Direction>{cout, "\n"});
        cout << endl;
    }
    return EXIT_SUCCESS;
}
