#include "agent.h"

#include <algorithm>
#include <array>
#include <string>
#include <iostream>

namespace escape_cat {


    Point Agent::choose_move(const Game& game, std::string& debug) {
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
        Point new_target = { RADIUS * std::cos(new_target_angle), RADIUS * std::sin(new_target_angle) };

        debug = "Going for target (" + std::to_string(new_target.x) + std::to_string(new_target.y);

        return new_target;
    }

    /**
     * The region of the boundary circle where the cat would
     * catch the mouse if it were to reach it within the specified
     * number of turns
     */
    [[nodiscard]] CircleArc Agent::danger_zone(const State& state, int n_turns) const {
        Radian center = state.cat.angle();
        Radian cat_reach = state.cat.compute_reach(n_turns);

        // We also must make sure to be at least the DISTANCE_THRESHOLD away from
        // the cat when we reach the boundary, which further extends this danger_zone
        // Since keeping our mouse's trajectory as straight as possible speeds up the
        // march towards the boundary, it is worth it to compute an exact lower bound
        // for the angle we need, which is straightforward from the formula for the
        // chordlenght that appears in the @CircleArc class.
        // Since we only need to do this computation once, let's go ahead and  make
        // that happen too.

        Radian arc_padding = []{
            if (m_arc_padding > 0.0) return m_arc_padding;

            return m_arc_padding = 2.0 * std::asin(0.08);
        }();

        return { RADIUS, center - cat_reach - arc_padding, center + cat_reach + arc_padding };
    }

    /**
     * Some care must be taken when outputting our choice since what is expected
     * is an element of the unit integer lattice! If our danger_zone and padding
     * computations were too sharp, rounding in the wrong direction here could
     * screw up all that good work...
     *
     * This is probably unneeded if everything else was golden, but I chose to
     * go ahead and make sure to pick the integral point amongst the corners which
     * is furtherst away from the cat.
     */
    /// TODO Just return something ready for output but don't include <iostream> in here...
    void Agent::output_choice(std::ostream& _out, const Point& p, const State& state) {
        int x0 = std::floor(p.x);
        int x1 = std::ceil(p.x);
        int y0 = std::floor(p.y);
        int y1 = std::ceil(p.y);

        std::array<PointI, 4> corners { PointI
            {x1, y0}, {x1, y1},
            {x0, y0}, {x1, y0}
        };

        std::array<int, 4> distance_to_cat_score { 0, 1, 2, 3 };

        // Sort the first set of indices to indicates which corners are furtherst away from the cat
        // Note the reverse inequality in the lambda to sort them in decreasing order of distance to the cat
        std::sort(distance_to_cat_score.begin(), distance_to_cat_score.end(),
                  [&corners, cat_pos=state.cat.get_point_position()](int a, int b)
                  { return distance2(corners[a], cat_pos) > distance2(corners[b], cat_pos); });

        PointI choice = corners[0];
        std::cout << choice.x << ' ' << choice.y << std::endl;
    }


} // namespace
