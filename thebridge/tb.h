#ifndef TB_H_
#define TB_H_

#include <array>
#include <cstdint>
#include <iosfwd>
#include <vector>

namespace tb {

constexpr size_t Max_length = 500;
constexpr size_t Max_depth = 50;
constexpr size_t Max_actions = 5;

using Key = uint32_t;

enum class Cell { Bridge,
    Hole };
using Road = std::array<std::vector<Cell>, 4>;

using Bikes = std::array<bool, 4>;

enum class Action {
    None = 0,
    Speed = 1,
    Jump = 2,
    Up = 3,
    Down = 4,
    Slow = 5
};

/**
 * The immutable part of the game.
 */
struct Params {
    Road road;
    int start_bikes;
    int min_bikes;
};

/**
 * The mutable part of the game.
 */
struct State {
    Key key;
    size_t pos;
    size_t speed;
    Bikes bikes { 0, 0, 0, 0 };
    int turn;
    State* prev;
};

/**
 * Class implementing the game simulation.
 */
class Game {
public:
    Game() = default;

    void init(std::istream&);
    void apply(const State&, Action a, State&) const;
    const std::vector<Action>& valid_actions(const State&) const;
    Params const* parameters() const;
    State const* state() const;
    void show(std::ostream& _out, const State& s) const;

    const Road& get_road() const { return parameters()->road; }
};

} // namespace tb

#endif // TB_H_
