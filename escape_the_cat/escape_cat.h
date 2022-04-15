#ifndef ESCAPE_CAT_H_
#define ESCAPE_CAT_H_

#include "utilities.h"

#include <cmath>
#include <iosfwd>
#include <map>

namespace escape_cat {

class Cat;
class Mouse;


class Mouse {
public:
    Mouse(const Point_Euc& pos, const Cat& cat)
        : m_cat(cat)
    {}

    void constexpr set_position(Point_Euc mouse);

    /**
     * The position of the mouse in Euclidean coordinates
     */
    [[nodiscard]] constexpr Point_Euc euclidean_coordinates() const;

    /**
     * The position of the mouse in Radial coordinates
     */
    [[nodiscard]] constexpr Point_Radial radial_coordinates() const;

    /**
     * The (hardcoded from problem rules) speed of the mouse
     */
    [[nodiscard]] constexpr double speed() const noexcept { return m_speed; };

private:
    Radian m_angle{ -1.0 };
    Point_Euc m_pos;
    static constexpr inline double m_speed = 10.0;
    const Cat& m_cat;
};


class Cat {
public:
    Cat(const Mouse& mouse_);

    /**
     * Set the position (in radians) from the euclidean coordinates of the cat.
     * Note that the point should always lie at distance 500 from the origin.
     */
    void constexpr set_position(Point_Euc cat);

    /**
     * Set the target according to the mouse's current position
     */
    void constexpr chase_mouse() noexcept;

    /**
     * The position of the cat in Euclidean coordinates
     */
    [[nodiscard]] constexpr Point_Euc euclidean_coords() const noexcept;

    /**
     * The position of the cat in radial coordinates
     */
    [[nodiscard]] constexpr Point_Euc radial_coords() const;

    /**
     * @speed_ is the number of points (pixels) the cat can traverse every turn.
     */
    void set_speed(NPixel speed) noexcept { m_speed = speed; }

    /**
     * When accessed from this, the result represents how much of an angle the
     * cat can sweep every turn.
     */
    [[nodiscard]] Radian speed() const noexcept { return m_speed; }

    /**
     * Return the current point on the boundary circle where the cat is headed.
     */
    [[nodiscard]] Radian target() const noexcept { return m_target; }

private:
    // How many radians can the cat traverse every turn
    Radian m_speed;
    Radian m_angle;
    Radian m_target;
    const Mouse& m_mouse;
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

    [[nodiscard]] std::pair<Radian, Radian> endpoints() const noexcept { return std::pair{m_r1, m_r2}; }

    /**
     * Check if the provided @angle is part of this arc
     */
    [[nodiscard]] bool contains(Radian angle) const noexcept;

    void set_endpoints(Radian r1, Radian r2) { m_r1 = r1; m_r2 = r2; }

private:
    double m_radius;
    Radian m_r1;
    Radian m_r2;
};


template<typename PointT>
struct Edge {
    using Point = PointT;

    Edge(Point* s, Point* t)
        : m_source{s}, m_target{t}
    {}

    private:
        Point* m_source;
        Point* m_target;
};

/**
 * A class to represent three arbitrary entities as a triangle
 */
template<typename PointT>
struct Triangle {
public:
    using Point = PointT;
    using Edge = Edge<PointT>;

    Triangle() = default;

    Triangle(const Point& p1, const Point& p2, const Point& p3)
        : m_point{{p1, p2, p3}}, m_edge{{ {p1,p2}, {p2,p3}, {p1,p3} }}
    {}

    void register_point(std::string name, Point point);
    void register_edge_name(std::string name, std::string_view source, std::string_view target);

    std::map<std::string, Point&> point_map;
    std::map<std::string, Edge&> edge_map;

    std::array<Point, 3> m_point;
    std::array<Edge, 3> m_edge;
};

/**
 * Stores the triangle making up the mouse's closest point on boundary,
 * the mouse's position, and the cat's position, long with the number
 * of turns elapsed.
 */
struct State {
    State() = default;

    Triangle<Point_Euc> OMC;
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

    // Accessor for the current state.
    [[nodiscard]] const State& state() const noexcept { return m_state; }

private:
    State m_state;
    double cat_speed;
    double mouse_speed;
};


extern bool operator==(const Cat& cat, const Cat& othercat);
extern bool operator==(const Mouse& cat, const Mouse& othercat);


} // namespace catescape



#endif // ESCAPE_CAT_H_
