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


void Cat::chase_mouse() noexcept {

}

void Cat::set_position(Point_Euc cat) {
    m_angle = std::atan2(cat.x, cat.y);
}

[[nodiscard]] Point_Euc Cat::euclidean_coords() const {
    return { RADIUS*std::cos(m_angle), RADIUS*std::sin(m_angle) };
}

    void Mouse::set_position(Point_Euc mouse) {
        m_angle = std::atan2(mouse.x, mouse.y);
        m_radius = std::sqrt(mouse.x * mouse.x + mouse.y * mouse.y);
    }

    [[nodiscard]] Point_Euc Mouse::euclidean_coordinates() const {
        return { m_radius*std::cos(m_angle), m_radius*std::sin(m_angle) };
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


bool operator==(const Cat& cat, const Cat& otherCat) {
    return
        std::abs(cat.angle() - otherCat.angle())
        + std::abs(cat.speed() - otherCat.speed()) < 2 * EPSILON;
}
bool operator==(const Mouse& mouse, const Mouse& otherMouse) {
    return mouse.euclidean_coordinates() == otherMouse.euclidean_coordinates();
}


} // namespace escape_cat
