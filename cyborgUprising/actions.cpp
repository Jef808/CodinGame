#include "actions.h"

#include <iostream>
#include <sstream>

namespace cyborg {

std::string Wait::to_string() const { return "WAIT"; }

std::string MoveTroops::to_string() const {
  std::stringstream ss;
  ss << "MOVE " << source << ' ' << target << ' ' << n_troops;
  return ss.str();
}

std::string SendBomb::to_string() const {
  std::stringstream ss;
  ss << "BOMB " << source << ' ' << target;
  return ss.str();
}

std::string IncreaseProd::to_string() const {
  std::stringstream ss;
  ss << "INC " << target;
  return ss.str();
}

std::ostream &operator<<(std::ostream &out, const Action &action) {
  switch (action.type) {
  case Action::Type::Wait:
    return out << action.data.wait.to_string();
  case Action::Type::Move:
    return out << action.data.move.to_string();
  case Action::Type::Bomb:
    return out << action.data.bomb.to_string();
  case Action::Type::Prod:
    return out << action.data.prod.to_string();
  default:
    throw std::runtime_error("Invalid action type");
  }
}

} // namespace cyborg
