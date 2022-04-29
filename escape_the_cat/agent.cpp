#include "agent.h"
#include "utilities.h"

#include <algorithm>
#include <array>
#include <sstream>
#include <iostream>

namespace escape_cat {


    Point_Euc Agent::choose_move(const Game& game, std::string& debug) {
        const State& state = game.state();

        static const int n_turns_extra = std::ceil(80 + 2 * RADIUS * std::sin(RADIUS / 2.0));

        double angle_diff = [&]{
            if (state.mouse.x < 0.0 && state.cat.x < 0.0) {
                const Point_Euc mod_mouse { -state.mouse.x, state.cat.y };
                const Point_Euc mod_cat { -state.cat.x, state.mouse.y };
                return angle_between(mod_cat, mod_mouse);
            }
            return angle_between(state.cat, state.mouse);
        }();

        Point_Radial mouse_radial = euc2rad(state.mouse);
        int n_turns_lb = std::ceil((RADIUS - mouse_radial.radius) / MOUSE_SPEED);
        int n_turns_for_cat = std::ceil(angle_diff) * game.cat_speed();

        double rotation_needed = std::max(0.0, static_cast<double>(n_turns_lb - n_turns_for_cat - n_turns_extra)) * (game.cat_speed() / RADIUS);

        Point_Euc new_target { RADIUS * std::cos(rotation_needed), RADIUS * std::sin(rotation_needed) };

        debug = "Target: (" + std::to_string(new_target.x) + ", " + std::to_string(new_target.y) + ")";
        return new_target;

        // CircleArc to_avoid = danger_zone(state, n_turns_lower_bound);

        // // It thus remain to make a choice in the non-fatal space.
        // // Since sharp turns slows down our progress, we just look for
        // // the target that will least disturb the straightness of our path.
        // // That is, we compute the angles those two changes of directions make
        // // at the current position of the mouse, and go with the one closest to a
        // // flat angle.
        // Radian old_mouse_angle = state.mouse.angle();
        // auto [choice_a, choice_b] = to_avoid.endpoints();

        // Radian choice_a_change = std::abs(old_mouse_angle - choice_a);
        // Radian choice_b_change = std::abs(old_mouse_angle - choice_b);

        // Radian new_target_angle =
        //     choice_a_change < choice_b_change
        //     ? choice_a
        //     : choice_b;
        // Point_Euc new_target = { RADIUS * std::cos(new_target_angle), RADIUS * std::sin(new_target_angle) };

        // debug = "Going for target (" + std::to_string(new_target.x) + std::to_string(new_target.y);

        // return new_target;
    }

    void Agent::format_choice_for_submission(const Point_Euc& choice, std::string_view debug_string, std::string& buffer) {
        std::ostringstream ss { buffer };

        PointI_Euc point{choice};

        ss << point.x << ' ' << point.y << debug_string.substr(27) << std::endl;
    }


} // namespace
