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
  struct Move {
    int source;
    int target;
    int n_troops;
  };
  std::optional<Move> move;
};

inline Action make_wait() { return Action{}; }
inline Action make_move(int source, int target, int n_troops) {
  return Action{std::make_optional(Action::Move{source, target, n_troops})};
}

std::ostream &operator<<(std::ostream &, const Action &);

} // namespace cyborg

#endif // GAME_H_
