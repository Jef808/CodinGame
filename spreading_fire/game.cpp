#include "game.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <functional>
#include <limits>
#include <set>
#include <tuple>


Point Game::coords(size_t ndx) const {
  return Point{int(ndx % m_width), int(ndx / m_width)};
}

int Game::duration_cut(size_t ndx) const {
  switch (m_cells[ndx].type()) {
    case Cell::Type::Tree: return Tree.DurationCut;
    case Cell::Type::House: return House.DurationCut;
    default: return std::numeric_limits<int>::infinity();
  }
}

int Game::duration_fire(size_t ndx) const {
    switch (m_cells[ndx].type()) {
      case Cell::Type::Tree: return Tree.DurationFire;
      case Cell::Type::House: return House.DurationFire;
      default: return std::numeric_limits<int>::infinity();
  }
}

namespace {

template<typename Cont>
class CellInserter {
  Cont& m_cont;
public:
  CellInserter(Cont& cont) : m_cont{cont} {}

  auto operator()(size_t n) {
    return [&, n](Cell::Type t) {
      return m_cont.emplace_back(t, Cell::Status::NoFire, n);
    };
  }
};

template<typename Cont>
inline CellInserter<Cont> make_cell_inserter(Cont& cont) { return CellInserter<Cont>(cont); }

}  // namespace

void Game::init_input(std::istream &is) {
  is >> Tree.DurationCut >> Tree.DurationFire >> Tree.Value;
  is >> House.DurationCut >> House.DurationFire >> House.Value;
  is >> m_width >> m_height;
  is >> m_fire_origin.x >> m_fire_origin.y;

  m_fire_progress.resize(size(), -1);
  m_bdry.reserve(size());
  m_outer_bdry.reserve(size());
  m_cells.reserve(size());

  std::string buf;
  auto cell_inserter = make_cell_inserter(m_cells);

  for (size_t y = 0; y < m_height; ++y) {
    is >> buf;
    for (size_t x = 0; x < m_width; ++x) {
      auto in = cell_inserter(index(x, y));
      switch (buf[x]) {
        case '#': in(Cell::Type::Safe); break;
        case '.': in(Cell::Type::Tree); break;
        case 'X': in(Cell::Type::House); break;
      }
    }
  }

  const size_t fire_origin_ndx = index(m_fire_origin.x, m_fire_origin.y);
  m_fire_progress[fire_origin_ndx] = 0;
  m_cells[fire_origin_ndx].set_on_fire();
  m_cooldown = 0;
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

void Game::get_bdry() {
  m_bdry.clear();
  m_outer_bdry.clear();
  std::set<size_t> outer_buf;
  for (size_t n = 0; n < size(); ++n) {
    if (m_cells[n].status() == Cell::Status::OnFire) {
      m_bdry.push_back(n);
      auto ofs = offsets(n);
      for (auto o : ofs) {
        // Only take candidates for cutting here (candidates
        // for propagating fire)
        if (m_cells[o].type() != Cell::Type::Safe &&
            m_cells[o].status() == Cell::Status::NoFire) {
          auto [_, ok] = outer_buf.insert(o);
          if (ok) {
            m_outer_bdry.push_back(o);
          }
        }
      }
    }
  }
}

void Game::expand_fire(size_t n) {
  for (auto of : offsets(n)) {
    if (is_flammable(of)) {
        m_cells[of].set_on_fire();
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
      m_cooldown = m_cells[n].cutting_countdown() + 1;
    }
  }
}
