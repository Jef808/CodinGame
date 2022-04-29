#ifndef ESCAPE_CAT_H_
#define ESCAPE_CAT_H_

#include "utilities.h"
#include <cmath>
#include <iosfwd>
#include <map>

namespace escape_cat {

/**
 * Stores the triangle making up the mouse's closest point on boundary,
 * the mouse's position, and the cat's position, long with the number
 * of turns elapsed.
 */
struct State {
    State() = default;

    enum class Status { ongoing, lost, won };

    Point_Euc mouse;
    Point_Euc cat;

    int n_turns{ 0 };
};


class Game {
public:
    // Get the cat's speed, along with the initial position
    // of both the cat and the mouse.
    Game(std::istream& _in);

    // If one chooses to update the state from the input provided
    // by the codingame interface.
    void update_state(std::istream& _in);

    // Does the same as @update_state but from the implementation of
    // the game's simulation.
    State step(const Point_Euc& action);

    [[nodiscard]] double cat_speed() const { return m_cat_speed; }
    [[nodiscard]] double mouse_speed() const { return m_mouse_speed; }

    [[nodiscard]] State::Status status() const;

    // Accessor for the current state.
    [[nodiscard]] const State& state() const noexcept { return m_state; }

private:
    State m_state {};
    double m_cat_speed;
    double m_mouse_speed;
};


} // namespace catescape



#endif // ESCAPE_CAT_H_
