#include "agent.h"

#include <algorithm>
#include <array>
#include <sstream>
#include <iostream>

namespace escape_cat {


    Point_Euc Agent::choose_move(const Game& game, std::string& debug) {
        const State& state = game.state();

        double dist_to_bdry = RADIUS - state.mouse.radius();
        int n_turns_lower_bound = std::ceil(dist_to_bdry / state.mouse.speed());

        CircleArc to_avoid = danger_zone(state, n_turns_lower_bound);

        // It thus remain to make a choice in the non-fatal space.
        // Since sharp turns slows down our progress, we just look for
        // the target that will least disturb the straightness of our path.
        // That is, we compute the angles those two changes of directions make
        // at the current position of the mouse, and go with the one closest to a
        // flat angle.
        Radian old_mouse_angle = state.mouse.angle();
        auto [choice_a, choice_b] = to_avoid.endpoints();

        Radian choice_a_change = std::abs(old_mouse_angle - choice_a);
        Radian choice_b_change = std::abs(old_mouse_angle - choice_b);

        Radian new_target_angle =
            choice_a_change < choice_b_change
            ? choice_a
            : choice_b;
        Point_Euc new_target = { RADIUS * std::cos(new_target_angle), RADIUS * std::sin(new_target_angle) };

        debug = "Going for target (" + std::to_string(new_target.x) + std::to_string(new_target.y);

        return new_target;
    }

    // /**
    //  * The region of the boundary circle where the cat would
    //  * catch the mouse if it were to reach it within the specified
    //  * number of turns
    //  */
    // [[nodiscard]] CircleArc Agent::danger_zone(const State& state, int n_turns) const {
    //     Radian center = state.cat.angle();
    //     Radian cat_reach = state.cat.compute_reach(n_turns);

    //     // We also must make sure to be at least the DISTANCE_THRESHOLD away from
    //     // the cat when we reach the boundary, which further extends this danger_zone
    //     // Since keeping our mouse's trajectory as straight as possible speeds up the
    //     // march towards the boundary, it is worth it to compute an exact lower bound
    //     // for the angle we need, which is straightforward from the formula for the
    //     // chordlenght that appears in the @CircleArc class.
    //     // Since we only need to do this computation once, let's go ahead and  make
    //     // that happen too.

    //     Radian arc_padding = []{
    //         if (m_arc_padding > 0.0) return m_arc_padding;

    //         return m_arc_padding = 2.0 * std::asin(0.08);
    //     }();

    //     return { RADIUS, center - cat_reach - arc_padding, center + cat_reach + arc_padding };
    // }

    void Agent::format_choice_for_submission(const Point_Euc& choice, std::string_view debug_string, std::string& buffer) {
        std::ostringstream ss { buffer };

        PointI_Euc point{choice};

        ss << point.x << ' ' << point.y << debug_string.substr(27);
    }


} // namespace
