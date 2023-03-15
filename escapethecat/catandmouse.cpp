#include "catandmouse.h"

#include <cmath>
#include <iostream>

namespace EscapeTheCat {

namespace {

inline std::complex<double> next_mouse(const CatAndMouse& cm,
                                       const std::complex<double>& target) {
    /** Localize target. */
    const auto localized_target = target - cm.mouse();
    /** Return mouse moved by localized target. */
    return cm.mouse() + MOUSE_SPEED * localized_target / abs(localized_target);
}

std::complex<double> next_cat(const CatAndMouse& cm) {
    const auto next_mouse_radius = abs(cm.mouse());
    const auto next_mouse_unit = cm.mouse() / next_mouse_radius;

    // Rotate cat's position so that the mouse points towards (-1, 0).
    auto normalized_cat = -cm.cat() * conj(next_mouse_unit);

    auto normalized_cat_angle = arg(normalized_cat);

    // +1 if normalized cat will go through the upper semi-circle, -1 otherwise.
    const int sign = static_cast<int>(
            std::round(normalized_cat_angle / std::abs(normalized_cat_angle)));

    // Adjust cat angle to reflect cat's movement.
    normalized_cat_angle += sign * (cm.cat_speed() / POOL_RADIUS);

    // Make sure cat stops at target if it is reached.
    if (normalized_cat_angle > M_PI || normalized_cat_angle < -M_PI) {
        normalized_cat_angle = sign * M_PI;
    }

    // Next cat normalized
    auto next_cat = std::polar(POOL_RADIUS, normalized_cat_angle);

    // Return unnormalized next cat
    next_cat *= -next_mouse_unit;
    return next_cat;
}

} // namespace

CatAndMouse::CatAndMouse(double cat_speed)
    : m_cat_speed{cat_speed} {
}

void CatAndMouse::input(std::istream& is) {
    int X, Y;
    is >> X >> Y;
    m_mouse.real(X);
    m_mouse.imag(Y);
    is >> X >> Y;
    m_cat.real(X);
    m_cat.imag(Y);
}

double CatAndMouse::score() const {
    // Cat position's rotated so that the mouse points towards (1, 0).
    const auto normalized_cat = m_cat * conj(m_mouse / abs(m_mouse));

    // Ratio of path completed by cat
    const auto cat_ratio = std::abs(arg(normalized_cat)) / M_PI;

    // Ratio of path completed by mouse
    const auto mouse_ratio = abs(m_mouse) / POOL_RADIUS;

    return cat_ratio + mouse_ratio;
}

void CatAndMouse::step(const std::complex<double>& target) {
    m_cat_prev = m_cat;
    m_mouse_prev = m_mouse;

    m_mouse = next_mouse(*this, target);
    m_cat = next_cat(*this);
}

void CatAndMouse::undo() {
    m_cat = m_cat_prev;
    m_mouse = m_mouse_prev;
}

CatAndMouse initialize(std::istream& is) {
    int cat_speed;
    is >> cat_speed;
    is.ignore();

    return CatAndMouse(cat_speed);
}

} // namespace EscapeTheCat
