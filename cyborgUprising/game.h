#ifndef GAME_H_
#define GAME_H_

#include <array>
#include <iosfwd>
#include <limits>
#include <optional>
#include <vector>

namespace cyborg {

constexpr const int INT_INFTY = 32000;

struct Node;
struct Edge;
struct Bomb;
struct Action;

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

struct Action {
  enum { Wait, Move, Bomb, Prod } type{Wait};
};

struct Wait : Action {
  Wait() : Action{Action::Wait} {}
};

struct MoveTroops : Action {
  MoveTroops(int _source, int _target, int _n_troops)
      : Action{Action::Move}, source{_source}, target{_target},
        n_troops{_n_troops} {}
  int source;
  int target;
  int n_troops;
};

struct SendBomb : Action {
  SendBomb(int _source, int _target)
      : Action{Action::Bomb}, source{_source}, target{_target} {}
  int source;
  int target;
};

struct IncreaseProd : Action {
  IncreaseProd(int _target) : Action{Action::Prod}, target{_target} {}
  int target;
};

inline Action make_wait() { return Wait{}; }
inline Action make_move(int source, int target, int n_troops) {
  return MoveTroops{source, target, n_troops};
}
inline Action make_bomb(int source, int target) {
  return SendBomb{source, target};
}
inline Action make_prod_increase(int target) { return IncreaseProd{target}; }

std::ostream &operator<<(std::ostream &, const Action &);

} // namespace cyborg

#endif // GAME_H_
