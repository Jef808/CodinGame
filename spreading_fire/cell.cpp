#include "cell.h"


#include <cassert>
#include <iostream>
#include <vector>


TreeT Tree;
HouseT House;


Cell::Cell(Type type, Status status, const size_t n)
    : m_type{type}, m_status{status}, m_index{n}
{

}


void Cell::start_cutting() {
    assert(m_type != Type::Safe && m_status != Cell::Status::OnFire);
    m_status = Status::Cutting;
    m_type = Type::Safe;
    m_cutting_countdown =
        m_type == Type::Tree ? Tree.DurationCut - 1: House.DurationCut - 1;
  }


void Cell::set_on_fire() {
    assert(m_type != Type::Safe);
    m_status = Status::OnFire;
    m_fire_countdown =
        m_type == Type::Tree ? Tree.DurationFire - 1: House.DurationFire - 1;
  }

void Cell::set_safe() { m_type = Type::Safe; }


bool Cell::update() {
    if (m_status == Status::OnFire) {
      if (--m_fire_countdown == 0) {
        m_status = Status::Burnt;
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

int Cell::value() const {
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
  return out;
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
  return out;
}
