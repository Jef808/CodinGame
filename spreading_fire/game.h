#ifndef GAME_H_
#define GAME_H_

#include "cell.h"
#include "point.h"

#include <array>
#include <iosfwd>
#include <vector>

/** One more than the largest possible index. */
constexpr size_t NULL_INDEX = 50 * 50 + 1;

struct Move {
  enum class Type { Wait, Cut };
  Type type;
  size_t index;
};

class Game {
public:
  using cell_iterator = std::vector<Cell>::const_iterator;

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
  Point coords(size_t ndx) const;
  size_t fire_origin() const { return index(m_fire_origin.x, m_fire_origin.y); }

  std::array<size_t, 4> offsets(size_t ndx) const;

  int duration_cut(size_t ndx) const;
  int duration_fire(size_t ndx) const;
  bool is_flammable(size_t ndx) const;

  const Cell& cell(size_t ndx) const { return m_cells[ndx]; }
  cell_iterator cells_begin() const { return m_cells.begin(); }
  cell_iterator cells_end() const { return m_cells.end(); }

  int value(size_t ndx) const { return m_cells[ndx].value(); }

  void get_bdry();

  friend class Agent;

private:
  size_t m_width;
  size_t m_height;
  Point m_fire_origin;
  int m_cooldown;
  std::vector<int> m_fire_progress;
  std::vector<size_t> m_bdry;
  std::vector<size_t> m_outer_bdry;
  std::vector<Cell> m_cells;
};


inline std::array<size_t, 4> Game::offsets(size_t n) const {
  return {n - m_width, n - 1, n + 1, n + m_width};
}

inline bool Game::is_flammable(size_t ndx) const {
  return m_cells[ndx].type() != Cell::Type::Safe
    && m_cells[ndx].status() == Cell::Status::NoFire;
  }

#endif // GAME_H_
