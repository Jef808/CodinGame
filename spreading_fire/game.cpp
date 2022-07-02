#include "game.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <functional>
#include <set>
#include <tuple>


Point Game::coords(size_t ndx) const {
  return Point{int(ndx % m_width), int(ndx / m_width)};
}

int Game::duration_cut(size_t ndx) const {
  return m_cells[ndx].type() == Cell::Type::Tree ? Tree.DurationCut
                                                 : House.DurationCut;
}

int Game::duration_fire(size_t ndx) const {
  return m_cells[ndx].type() == Cell::Type::Tree ? Tree.DurationFire
                                                 : House.DurationFire;
}

void Game::init_input(std::istream &is) {
  using namespace std::placeholders;

  is >> Tree.DurationCut >> Tree.DurationFire >> Tree.Value;
  is >> House.DurationCut >> House.DurationFire >> House.Value;
  is >> m_width >> m_height;
  is >> fire_origin.x >> fire_origin.y;

  m_fire_progress.resize(size(), -1);
  m_bdry.reserve(size());
  m_outer_bdry.reserve(size());
  m_cells.reserve(size());

  std::string buf;

  auto type = Cell::Type::Tree;
  auto CellCtor = [&, s=Cell::Status::NoFire](auto t, auto n) {
    return m_cells.emplace_back(t, s, n);
  };
  auto CCtor = std::bind(CellCtor, std::ref(type), _1);

  for (size_t y = 0; y < m_height; ++y) {
    is >> buf;
    for (size_t x = 0; x < m_width; ++x) {

      switch (buf[x]) {
        case '#': type = Cell::Type::Safe; break;
        case '.': type = Cell::Type::Tree; break;
        case 'X': type = Cell::Type::House; break;
      }

      const Cell& cell = CCtor(index(x, y));
    }
  }

  const size_t fire_origin_ndx = index(fire_origin.x, fire_origin.y);
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
      m_cooldown = m_cells[n].cutting_countdown() + 1;
    }
  }
}
