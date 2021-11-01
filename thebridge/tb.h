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
    size_t pos{ 0 };
    size_t speed{ 0 };
    Bikes bikes { 0, 0, 0, 0 };
    int turn{ 0 };
    State* prev{ nullptr };
};

/**
 * Class implementing the game simulation.
 */
class Game {
public:
    Game() = default;

    /// Initialize the game from an input stream
    void init(std::istream&);

    /// Apply the action to cur_state and store the result in `next_state'
    void apply(const State& cur_state, Action action, State& next_state) const;

    /// Generate the list of valid actions for `state'
    const std::vector<Action>& valid_actions(const State& state) const;

    /// Get the position of the last hole on the road
    int find_last_hole() const;

    /// Get the game's parameters
    Params const* parameters() const;

    /// Get the game's root state
    State const* state() const;

    /// Send a representation of `state' to the output stream
    void show(std::ostream& _out, const State& state) const;

    /// Get the road
    const Road& get_road() const { return parameters()->road; }
};

} // namespace tb

#endif // TB_H_
