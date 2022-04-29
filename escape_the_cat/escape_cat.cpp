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

    Game::Game(std::istream& _in) {
        _in >> m_cat_speed;
    }

    void Game::update_state(std::istream& _in) {
        double m_x, m_y, c_x, c_y;
        _in >> m_x >> m_y >> c_x >> c_y;
        m_state.mouse.x = m_x;
        m_state.mouse.y = m_y;
        m_state.cat.x = c_x;
        m_state.cat.y = c_y;
        ++m_state.n_turns;
    }

    [[nodiscard]] State::Status Game::status() const {
        using Status = State::Status;
        Status status = Status::ongoing;

        // Ran out of time
        if (m_state.n_turns > 350)
            return Status::lost;

        // Once we reached the boundary
        if (m_state.mouse.x * m_state.mouse.x + m_state.mouse.y * m_state.mouse.y > RADIUS * RADIUS - 1.0f) {

            return std::abs(angle_between(m_state.cat, m_state.mouse)) > ANGLE_THRESHOLD ? State::Status::won : State::Status::lost;
        }

        return State::Status::ongoing;
    }




} // namespace escape_cat
