#ifndef ACTIONS_H_
#define ACTIONS_H_


#include <string>

namespace cyborg {

// Actions data:
struct Wait {
  std::string to_string() const;
};

struct MoveTroops {
  std::string to_string() const;
  int source;
  int target;
  int n_troops;
};

struct SendBomb {
  std::string to_string() const;
  int source;
  int target;
  SendBomb(int s, int t) : source{s}, target{t} {}
};

struct IncreaseProd {
  std::string to_string() const;
  int target;
};

struct Action {
  enum class Type { Wait, Move, Bomb, Prod };
  union Data {
    Wait wait;
    MoveTroops move;
    SendBomb bomb;
    IncreaseProd prod;
  };
  Type type;
  Data data{Wait{}};
};

template <Action::Type T, typename... Args>
inline Action make_action(Args &&...args);

template <> inline Action make_action<Action::Type::Wait>() {
  Action action{Action::Type::Wait};
  action.data.wait = Wait{};
  return action;
}

template <>
inline Action make_action<Action::Type::Move>(const int &s, const int &t,
                                              const int &n) {
  Action action{Action::Type::Move};
  action.data.move = MoveTroops{s, t, n};
  return action;
}

template <>
inline Action make_action<Action::Type::Bomb>(const int &s, const int &t) {
  Action action{Action::Type::Move};
  action.data.bomb = SendBomb{s, t};
  return action;
}
template <> inline Action make_action<Action::Type::Prod>(const int &t) {
  Action action{Action::Type::Wait};
  action.data.prod = IncreaseProd{t};
  return action;
}

std::ostream &operator<<(std::ostream &, const Action&);

} // namespace cyborg

#endif // ACTIONS_H_
