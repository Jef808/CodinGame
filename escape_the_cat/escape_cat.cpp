#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <sstream>
#include <utility>
#include <random>


#include "escape_cat.h"


namespace escape_cat {

    namespace {

constexpr double degree_2_radian(long degree) {
    return static_cast<double>(degree) * M_PI / 180.0;
};

void normalize_angle(double& angle, double min=-M_PI, double max=M_PI) {
    if (angle < min)
        angle += 2.0*M_PI;
    else if (angle > max)
        angle -= 2.0*M_PI;

    assert(angle > min - EPSILON
           && "Cat angle is below min after normalizing");

    assert(angle < max + EPSILON
           && "Cat angle is above max after normalizing");
}
    } // unnamed namespace



void Cat::set_position(Point cat) {
    m_angle = std::atan2(cat.x, cat.y);
}


[[nodiscard]] Point Cat::get_point_position() const {
    return { RADIUS*std::cos(m_angle), RADIUS*std::sin(m_angle) };
}

void Cat::set_speed(NPixel speed) noexcept {
    m_speed = static_cast<double>(speed) / RADIUS;
}
    [[nodiscard]] Radian Cat::speed() const noexcept { return m_speed; }

    [[nodiscard]] Radian Cat::angle() const noexcept { return m_angle; }

    [[nodiscard]] Radian Cat::compute_reach(int n_turns) const noexcept {
        return m_speed * n_turns;
    }

    void Mouse::set_position(Point mouse) {
        m_angle = std::atan2(mouse.x, mouse.y);
        m_radius = std::sqrt(mouse.x * mouse.x + mouse.y * mouse.y);
    }

    [[nodiscard]] Point Mouse::point_position() const {
        return { m_radius*std::cos(m_angle), m_radius*std::sin(m_angle) };
    }

    [[nodiscard]] double Mouse::radius() const noexcept {
        return m_radius;
    }

    [[nodiscard]] Radian Mouse::angle() const noexcept {
        return m_angle;
    }

    [[nodiscard]] double Mouse::speed() const noexcept {
        return m_speed;
    }

    CircleArc::CircleArc(NPixel radius, Radian r1, Radian r2)
        : m_radius{radius}, m_r1{r1}, m_r2{r2}
    {
        assert(radius > EPSILON
               && "Calling constructor of CircleArc with radius <=0 is not permitted");
    }

    [[nodiscard]] Radian CircleArc::extent() const noexcept {
        return std::abs(m_r1 - m_r2);
    }

    [[nodiscard]] double CircleArc::arclength() const noexcept {
        return std::abs(extent() * m_radius);
    }

    [[nodiscard]] double CircleArc::chordlength() const noexcept {
        return std::abs(2.0 * m_radius * std::sin(extent() / 2.0));
    }

        [[nodiscard]] std::pair<Radian, Radian> CircleArc::endpoints() const noexcept {
            return std::make_pair(m_r1, m_r2);
        }

    [[nodiscard]] bool CircleArc::contains(Radian angle) const noexcept {
        Radian a = std::min(m_r1, m_r2);
        Radian b = std::max(m_r1, m_r2);

        return angle > a - EPSILON && angle < b + EPSILON;
    }

    void CircleArc::set_endpoints(Radian r1, Radian r2) {
        assert(r1 != r2
               && "A degenerate Circle Arc (r1 == r2) is not allowed");
        m_r1 = r1;
        m_r2 = r2;
    }


    void Game::init(std::istream& _in) {
        int speed; _in >> speed;
        m_state.cat.set_speed(speed);
    }

    void Game::update_state(std::istream& _in) {
        double m_x, m_y, c_x, c_y;
        _in >> m_x >> m_y >> c_x >> c_y;
        m_state.mouse.set_position({m_x, m_y});
        m_state.cat.set_position({c_x, c_y});
        ++m_state.n_turns;
    }

    [[nodiscard]] const State& Game::state() const noexcept { return m_state; }

    [[nodiscard]] State::Status Game::status() {
        using Status = State::Status;
        Status status = Status::ongoing;

        // Ran out of time
        if (m_state.n_turns > 350)
            status = Status::lost;

        // Once we reached the boundary
        else if (m_state.mouse.radius() > RADIUS - EPSILON) {

            Radian mouse_angle = m_state.mouse.angle();
            Radian cat_angle = m_state.cat.angle();

            CircleArc arc { RADIUS, mouse_angle, cat_angle };
            double euc_dist = arc.chordlength();

            status = euc_dist < DISTANCE_THRESHOLD ? Status::lost : Status::won;
        }

        return status;
    }

    // FIXME
    State Game::step(const Point& action) {

        State result = m_state;

        Point current_pos = m_state.mouse.point_position();
        Point next_direction = action - current_pos;
        double norm = std::sqrt(radius2(next_direction));
        Point new_mouse_position = rescale(next_direction, 10.0 / norm);

        result.mouse.set_position(new_mouse_position);

        Radian new_mouse_angle = std::atan2(new_mouse_position.x, new_mouse_position.y);
        Radian cat_angle = m_state.cat.angle();
        Point new_cat_target = rescale(new_mouse_position, RADIUS / 10.0);

        Radian angle_diff = new_mouse_angle - cat_angle;
        if (std::abs(angle_diff) < M_PI) { // Meaning we are on the shorted side of the circle
            Radian new_cat_angle = angle_diff > 0.0 ? cat_angle + m_state.cat.speed() : cat_angle - m_state.cat.speed();
            result.cat.set_position({ RADIUS*std::cos(new_cat_angle), RADIUS*std::sin(new_cat_angle) });
        }

        return result;
    }


bool Point::operator!=(const Point& other) {
    std::cerr << "Warning: Implementation of comparison between double-valued points sloppily compares the result of rounding down" << std::endl;
    PointI rndd_p = *this;
    Point other_ = other;
    PointI rndd_otherp = other_;
    return rndd_p != rndd_otherp;
}

bool operator==(const Point& point, const Point& otherPoint) {
    return std::floor(point.x) == std::floor(otherPoint.x) && std::floor(point.y) == std::floor(otherPoint.y);
}

bool operator==(const Cat& cat, const Cat& otherCat) {
    return
        std::abs(cat.angle() - otherCat.angle())
        + std::abs(cat.speed() - otherCat.speed()) < 2 * EPSILON;
}
bool operator==(const Mouse& mouse, const Mouse& otherMouse) {
    return std::abs(mouse.angle() - otherMouse.angle())
        + (distance2(mouse.point_position(), otherMouse.point_position())) < 2 * EPSILON;
}


} // namespace escape_cat
