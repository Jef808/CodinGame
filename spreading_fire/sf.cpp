#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <iterator>
#include <limits>
#include <ostream>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace {
template <typename E, typename I = std::enable_if_t<std::is_enum_v<E>,
                                                    std::underlying_type_t<E>>>
I to_int(E e) {
  return static_cast<I>(e);
}
} // namespace

/** One more than the largest possible index. */
constexpr size_t NULL_INDEX = 50 * 50 + 1;

/**
 * @Unknown  Initialization value for distances.
 * @Zero  When the cell is on fire.
 * @One  When the cell will catch on fire on the next round.
 * @Never  When the cell can never catch on fire.
 */
enum class FireDistance {
  Never = -1,
  Zero = 0,
  One = 1,
  Unknown = std::numeric_limits<unsigned int>::infinity(),
};

inline FireDistance operator++(FireDistance fd) {
  return FireDistance(to_int(fd) + 1);
}
inline FireDistance operator+(FireDistance fd, int n) {
  return FireDistance(to_int(fd) + n);
}
inline FireDistance &operator+=(FireDistance &fd, int n) {
  fd = FireDistance(to_int(fd) + n);
  return fd;
}
inline bool operator<(FireDistance a, FireDistance b) {
  return to_int(a) < to_int(b);
}

struct Point {
  int x;
  int y;
};

inline int manhattan_distance(const Point &a, const Point &b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

inline int euclidean_distance_2(const Point &a, const Point &b) {
  return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

struct Segment {
  Point source;
  Point target;
};

enum class Direction { Same = 0, Outwards = 1, Inwards = -1 };

struct {
  int DurationCut;
  int DurationFire;
  int Value;
} Tree{};

struct {
  int DurationCut;
  int DurationFire;
  int Value;
} House{};

/**
 * Store the Type and Status of a cell by combining both enums
 * into one unsigned 8 bit integers with a bitwise OR operation.
 */
class Cell {
public:
  enum class Type : uint8_t { Safe = 1, Tree = 2, House = 4 };
  enum class Status : uint8_t {
    NoFire = 1,
    OnFire = 2,
    Burnt = 4,
    Cut = 8,
    Cutting = 16
  };

  Cell(Type type, Status status, const Point &p)
      : m_type{type}, m_status{status}, m_coords{p} {}

  /** Prevent any construction of invalid cell. */
  Cell() = delete;

  /** Return the Type part of the cell's state. */
  Status status() const { return m_status; }

  /** Return the Status part of the cell's state. */
  Type type() const { return m_type; }

  /** Set the status flag. */
  // void set_status(Status s) {
  //     assert(m_type != Type::Safe);
  //     m_status = s; }

  /** Change the type to safe (cell is burned or cut) */
  void start_cutting() {
    assert(m_type != Type::Safe && m_status != Cell::Status::OnFire);
    m_type = Type::Safe;
    m_status = Status::Cutting;
    m_cutting_countdown =
        m_type == Type::Tree ? Tree.DurationCut : House.DurationCut;
  }

  /** Set the cell on fire */
  void set_on_fire() {
    assert(m_type != Type::Safe);
    // m_type = Type::Safe;
    m_status = Status::OnFire;
    m_fire_countdown =
        m_type == Type::Tree ? Tree.DurationFire : House.DurationFire;
  }

  /** The time before the cell is burnt, or -1 if no fire. */
  int fire_countdown() const { return m_fire_countdown; }

  /** The time before the cell is cut, or -1 if not cutting. */
  int cutting_countdown() const { return m_cutting_countdown; }

  /**
   * Decrement any necessary countdown, set type and status if burnt
   *  or cut.
   * In case one of the two countdowns gets to 0, return true.
   */
  bool update() {
    if (m_status == Status::OnFire) {
      if (--m_fire_countdown == 0) {
        m_status = Status::Burnt;
        m_fire_countdown = -1;
        return true;
      }
    } else if (m_status == Status::Cutting) {
      if (--m_cutting_countdown == 0) {
        m_status = Status::Cut;
        return true;
      }
    }
    return false;
  }

  /** Return the value, or 0 if the cell is consumed. */
  int value() const {
    if (m_status == Status::NoFire) {
      switch (m_type) {
      case Type::Safe:
        return 0;
      case Type::Tree:
        return Tree.Value;
      case Type::House:
        return House.Value;
      }
    }
    return 0;
  }

private:
  /** Both the type and status are encoded in an 8-bit integer */
  Type m_type;

  /** Whether the cell is on fire or not */
  Status m_status;

  /** The cell's coordinates */
  Point m_coords;

  /** The cell's countdown until it gets Burnt status */
  int m_fire_countdown{-1};

  /** The cell's countdown until it gets Cut status */
  int m_cutting_countdown{-1};
};

std::ostream &operator<<(std::ostream &out, Cell::Status s) {
  switch (s) {
  case Cell::Status::NoFire:
    return out << "NoFire";
  case Cell::Status::OnFire:
    return out << "OnFire";
  case Cell::Status::Burnt:
    return out << "Burnt";
  case Cell::Status::Cut:
    return out << "Cut";
  case Cell::Status::Cutting:
    return out << "Cutting";
  }
}

std::ostream &operator<<(std::ostream &out, Cell::Type t) {
  switch (t) {
  case Cell::Type::Safe:
    return out << "Safe";
  case Cell::Type::Tree:
    return out << "Tree";
  case Cell::Type::House:
    return out << "House";
  }
}

struct Move {
  enum class Type { Wait, Cut };
  Type type;
  size_t index;
};

class Game {
public:
  Game() = default;

  void init_input(std::istream &is);
  void turn_input(std::istream &is);

  bool test_against_turn_input();

  void apply(const Move &move);
  void expand_fire(size_t n);

  size_t width() const { return m_width; }
  size_t height() const { return m_height; }
  size_t size() const { return m_width * m_height; }
  size_t index(size_t x, size_t y) const { return x + m_width * y; }
  Point coords(size_t ndx) const {
    return Point{int(ndx % m_width), int(ndx / m_width)};
  }
  std::array<size_t, 4> offsets(size_t ndx) const;

  int duration_cut(size_t ndx) const {
    return m_cells[ndx].type() == Cell::Type::Tree ? Tree.DurationCut
                                                   : House.DurationCut;
  }
  int duration_fire(size_t ndx) const {
    return m_cells[ndx].type() == Cell::Type::Tree ? Tree.DurationFire
                                                   : House.DurationFire;
  }
  int value(size_t ndx) const { return m_cells[ndx].value(); }

  void get_bdry();

  friend class Agent;

private:
  size_t m_width;
  size_t m_height;
  Point fire_origin;
  int m_cooldown;
  Move m_last_move;
  std::vector<int> m_fire_progress;
  std::vector<Cell> m_cells;
  std::vector<size_t> m_bdry;
  std::vector<size_t> m_outer_bdry;
};

void Game::init_input(std::istream &is) {
  is >> Tree.DurationCut >> Tree.DurationFire >> Tree.Value;
  is >> House.DurationCut >> House.DurationFire >> House.Value;
  is >> m_width >> m_height;
  is >> fire_origin.x >> fire_origin.y;
  m_fire_progress.resize(size(), -1);
  m_cells.reserve(size());
  std::string buf;
  for (size_t y = 0; y < m_height; ++y) {
    is >> buf;
    for (size_t x = 0; x < m_width; ++x) {
      switch (buf[x]) {
      case '#':
        m_cells.emplace_back(Cell::Type::Safe, Cell::Status::NoFire,
                             Point{(int)x, (int)y});
        break;
      case '.':
        m_cells.emplace_back(Cell::Type::Tree, Cell::Status::NoFire,
                             Point{(int)x, (int)y});
        break;
      case 'X':
        m_cells.emplace_back(Cell::Type::House, Cell::Status::NoFire,
                             Point{(int)x, (int)y});
        break;
      }
    }
  }
  const size_t fire_origin_ndx = index(fire_origin.x, fire_origin.y);
  m_fire_progress[fire_origin_ndx] = 0;
  m_cells[fire_origin_ndx].set_on_fire();
  m_bdry.reserve(size());
  m_outer_bdry.reserve(size());
}

void Game::turn_input(std::istream &is) {
  is >> m_cooldown;
  for (size_t y = 0; y < m_height; ++y) {
    for (size_t x = 0; x < m_width; ++x) {
      size_t n = index(x, y);
      is >> m_fire_progress[n];
    }
  }
}

bool Game::test_against_turn_input() {
  auto it = std::find(m_fire_progress.begin(), m_fire_progress.end(), -1);
  if (it == m_fire_progress.end()) {
    std::cerr << "FOREST HAS BURNED" << std::endl;
    return false;
  }

  bool okay = true;
  for (size_t y = 0; y < m_height; ++y) {
    for (size_t x = 0; x < m_width; ++x) {
      size_t n = index(x, y);
      int fp = m_fire_progress[n];

      Cell::Type type = m_cells[n].type();
      Cell::Status status = m_cells[n].status();

      // Cell is a safe cell.
      if (fp == -2) {
        if (type != Cell::Type::Safe) {
          std::cerr << "Cell " << x << ' ' << y << " has type " << type
                    << " instead of Safe" << std::endl;
          okay = false;
        }
      }

      // Cell is a Tree or House with no fire
      else if (fp == -1) {
        if (type == Cell::Type::Safe) {
          std::cerr << "Cell " << x << ' ' << y << " has type " << type
                    << " instead of House or Tree" << std::endl;
          okay = false;
        } else if (status != Cell::Status::NoFire) {
          std::cerr << "Cell " << x << ' ' << y << " has status " << status
                    << " instead of NoFire" << std::endl;
          okay = false;
        }
      }

      // Cell is on fire
      else if (fp >= 0 && fp < duration_fire(n)) {
        if (status != Cell::Status::OnFire) {
          std::cerr << "Cell" << x << ' ' << y << " has status" << status
                    << " instead of OnFire" << std::endl;
          okay = false;
        }
      }

      // Cell has burned
      else if (fp == duration_fire(n)) {
        if (status != Cell::Status::Burnt) {
          std::cerr << "Cell" << x << ' ' << y << " has status" << status
                    << " instead of Burnt" << std::endl;
          okay = false;
        }
      }
    }
  }
  return okay;
}

inline std::array<size_t, 4> Game::offsets(size_t n) const {
  return {n - m_width, n - 1, n + 1, n + m_width};
}

void Game::get_bdry() {
  m_bdry.clear();
  m_outer_bdry.clear();
  std::set<size_t> outer_buf;
  for (size_t n = 0; n < size(); ++n) {
    if (m_cells[n].status() == Cell::Status::OnFire) {
      auto ofs = offsets(n);
      for (auto o : ofs) {
        // Only take candidates for cutting here (candidates
        // for propagating fire)
        if (m_cells[o].type() != Cell::Type::Safe &&
            m_cells[o].status() == Cell::Status::NoFire) {
          auto [_, ok] = outer_buf.insert(o);
          if (ok) {
            m_bdry.push_back(n);
            m_outer_bdry.push_back(o);
          }
        }
      }
    }
  }
}

void Game::expand_fire(size_t n) {
  for (auto of : offsets(n)) {
    if (m_cells[of].type() != Cell::Type::Safe) {
      if (m_cells[of].status() == Cell::Status::NoFire) {
        m_cells[of].set_on_fire();
      }
    }
  }
}

void Game::apply(const Move &move) {
  if (move.type == Move::Type::Wait) {
    if (m_cooldown > 0) {
      get_bdry();
      assert(m_cooldown > 0 ||
             m_outer_bdry.empty() &&
                 "Failed when applying 'Wait' while cooldown is zero");
    }
  } else if (move.type == Move::Type::Cut) {
    assert(m_cooldown == 0 && "Failed at non-zero cutting countdown");
    assert(m_cells[move.index].type() != Cell::Type::Safe &&
           "Failed when trying to cut a Safe cell.");
    assert(m_cells[move.index].status() == Cell::Status::NoFire &&
           "Failed when trying to cut a cell on fire.");

    m_cells[move.index].start_cutting();
  }

  for (size_t n = 0; n < size(); ++n) {
    if (m_cells[n].update()) {
      switch (m_cells[n].status()) {
      case Cell::Status::Burnt:
        expand_fire(n);
        break;
      case Cell::Status::Cut:
        m_cooldown = 0;
        break;
      default:
        assert(false && "Failed when update() returned true"
                        "without the status being Burnt or Cut");
      }
    }

    if (m_cells[n].status() == Cell::Status::Cutting) {
      m_cooldown = m_cells[n].cutting_countdown();
    }
  }
}

class Agent {
public:
  Agent(Game &game) : m_game{game} {}

  Move choose_move() {
    if (m_game.m_cooldown > 0) {
      return {Move::Type::Wait, NULL_INDEX};
    }
    get_moves_candidates();
    if (m_candidates.empty()) {
      return {Move::Type::Wait, NULL_INDEX};
    }
    // rank_candidates();
    return {Move::Type::Cut, m_candidates[0]};
  }

  double score_function(size_t n) {
    constexpr double lifetime_cost = 0.3;
    constexpr double value_weight = 0.7;
    size_t quickest_parent = NULL_INDEX;
    int shortest_time_before_fire = std::numeric_limits<int>::infinity();
    for (auto p : m_game.offsets(n)) {
      if (m_game.m_cells[p].status() == Cell::Status::OnFire) {
        if (m_game.m_cells[p].fire_countdown() < shortest_time_before_fire) {
          shortest_time_before_fire = m_game.m_cells[p].fire_countdown();
          quickest_parent = p;
        }
      }
    }

    assert(quickest_parent != NULL_INDEX);

    const double weighted_lifetime_cost =
        lifetime_cost * (shortest_time_before_fire);

    const size_t p = quickest_parent;
    auto [px, py] = m_game.coords(p);
    auto [x, y] = m_game.coords(n);
    int value_ray = 0;

    auto at_edge = [&](int x, int y) {
      return x == 0 || x == m_game.m_width - 1 || y == 0 ||
             y == m_game.m_height - 1;
    };

    int i = 0;
    int cx = x + i * (x - px);
    int cy = y + i * (y - py);
    while (not at_edge(cx, cy)) {
      value_ray += m_game.value(m_game.index(cx, cy));
      ++i;
    }

    const double weighted_avg_value_ray = value_weight * value_ray / (i + 1);

    return weighted_lifetime_cost + weighted_avg_value_ray;
  }

  void rank_candidates() {
    std::sort(m_candidates.begin(), m_candidates.end(), [&](auto a, auto b) {
      return score_function(a) < score_function(b);
    });
  }

  /**
   * Look at cells right next to the fire boundary.
   *
   * Note that get_bdry() is implemented so that the outer boundary
   * only contains flammable cells with no fire.
   */
  void get_moves_candidates() {
    m_game.get_bdry();
    m_candidates = m_game.m_outer_bdry;
  }

private:
  Game &m_game;
  std::vector<size_t> m_candidates;
};

int main(int argc, char *argv[]) {
  Game game;
  game.init_input(std::cin);
  Agent agent(game);

  while (true) {
    game.turn_input(std::cin);
    //bool okay = game.test_against_turn_input();
    //assert(okay);

    Move move = agent.choose_move();
    game.apply(move);

    if (move.type == Move::Type::Wait) {
      std::cout << "WAIT" << std::endl;
    } else {
      auto [x, y] = game.coords(move.index);
      std::cout << x << ' ' << y << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
