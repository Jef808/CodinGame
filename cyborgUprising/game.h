#ifndef GAME_H_
#define GAME_H_

#include <array>
#include <iosfwd>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cyborg {

constexpr const int INT_INFTY = 32000;

struct Node;
struct Edge;
struct Bomb;

struct Action;
std::ostream &operator<<(std::ostream &, const cyborg::Action &);

class Game {
public:
  Game() = default;

  /**
   * Get the game's global graph from an istream.
   */
  void init(std::istream &is);

  /**
   * Update the game's state from an istream
   * and increment the nb of turns counter.
   */
  void turn_update(std::istream &is);

  /** Accessors */
  const std::vector<Node> &nodes() const { return m_nodes; }
  const std::vector<std::vector<Edge>> &edges() const { return m_edges; }
  int turn() const { return n_turns; }

private:
  std::vector<Node> m_nodes;
  std::vector<std::vector<Edge>> m_edges;
  std::vector<Bomb> m_bombs;
  int n_turns{0};

  /**
   * Reset nodes and edges data.
   */
  void reset();
};

enum class Owner { None, Me, Them };

struct Node {
  int id{-1};
  Owner owner{Owner::None};
  int n_troops{0};
  int production{0};
};

struct Edge {
  int distance{INT_INFTY};
  int n_troops{0};
  Owner troops_owner{Owner::None};
  int turns_until_arrival{INT_INFTY};
};

struct Bomb {
  int id{-1};
  Owner owner{Owner::None};
  int turns_until_explode{INT_INFTY};
  int source{-1};
  std::vector<int> maybe_targets;
};

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

} // namespace cyborg

#endif // GAME_H_
