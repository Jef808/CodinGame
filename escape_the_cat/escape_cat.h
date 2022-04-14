#ifndef ESCAPE_CAT_H_
#define ESCAPE_CAT_H_

#include "utilities.h"

#include <cmath>
#include <iosfwd>


namespace escape_cat {


class Cat {
public:
    Cat() = default;

    /**
     * Set the position (in radians) from the euclidean coordinates of the cat.
     */
    void set_position(Point cat);

    /**
     * Get the x-y coordinates of the cat from the stored Radian angle.
     */
    [[nodiscard]] Point get_point_position() const;

    /**
     * @speed_ is the number of points the cat can traverse every turn.
     *
     * We convert this into a Radian angle indicating how fast the cat
     * can move along the boundary circle:
     *
     * Use theta_res / (2pi) = speed_ / (2piR) which lets us do this in one line.
     */
    void set_speed(NPixel speed) noexcept;
    [[nodiscard]] Radian speed() const noexcept;

    [[nodiscard]] Radian angle() const noexcept;

    [[nodiscard]] Radian compute_reach(int n_turns) const noexcept;

private:
    // How much the cat can change its angle in one turn
    Radian m_speed;
    Radian m_angle;
};

class Mouse {
public:
    Mouse() = default;

    void set_position(Point mouse);

    [[nodiscard]] Point point_position() const;

    [[nodiscard]] double radius() const noexcept;

    [[nodiscard]] Radian angle() const noexcept;

    [[nodiscard]] double speed() const noexcept;

private:
    Radian m_angle;
    double m_radius;
    double m_speed = 10.0;
};

struct State {
    State() = default;

    Cat cat;
    Mouse mouse;
    int n_turns{ 0 };

    enum class Status { ongoing, won, lost };
    Status status{ Status::ongoing };
};


/**
 * We don't bother keeping the endpoints "ordered".
 *
 * First, this lives on a circle so there is no canonical ordering of the points.
 * Second, if we are only interested in the arclength and chordlength computations
 * bellow, note that swapping the endpoints only introduces a minus sign in the result.
 */
class CircleArc {
public:
    CircleArc() = default;
    CircleArc(double radius, Radian r1, Radian r2);

    [[nodiscard]] Radian extent() const noexcept;

    [[nodiscard]] double arclength() const noexcept;

    [[nodiscard]] double chordlength() const noexcept;

    [[nodiscard]] std::pair<Radian, Radian> endpoints() const noexcept;

    /**
     * Check if the provided @angle is part of this arc
     */
    [[nodiscard]] bool contains(Radian angle) const noexcept;

    void set_endpoints(Radian r1, Radian r2);

private:
    double m_radius;
    Radian m_r1;
    Radian m_r2;
};


class Game {
public:
    Game() = default;

    void init(std::istream& _in);

    void update_state(std::istream& _in);

    [[nodiscard]] const State& state() const noexcept;

    [[nodiscard]] State::Status status();

    State step(const Point& action);

private:
    State m_state;
};

class Evaluation {
public:
    explicit Evaluation(const Game& game) : m_game{game} { }



private:
    const Game& m_game;
};


extern bool operator==(const Cat& cat, const Cat& othercat);
extern bool operator==(const Mouse& cat, const Mouse& othercat);


} // namespace catescape



#endif // ESCAPE_CAT_H_
