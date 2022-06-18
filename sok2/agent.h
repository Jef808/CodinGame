#ifndef AGENT_H_
#define AGENT_H_

#include "types.h"
#include "io.h"
#include "utils.h"

#include <algorithm>
#include <cassert>

namespace sok2 {


struct Interval {
    int m;
    int M;
    int size() const { return M - m + 1; }
};
struct Rectangle {
    Interval int_x;
    Interval int_y;
};


class Agent {
public:
    Agent(const Game& game)
        : m_game{ game }
        , rect{ {0, game.building.width - 1}, {0, game.building.height - 1} }
        , prev_pos{ game.current_pos }
    {}

    Window choose_move() {
        Window ret{ bomb_x, bomb_y };

        /* After find the bomb, we simply pick it. */
        if (found_x() && found_y()) {
            return ret;
        }

        const Interval& interval = found_x()
            ? rect.int_y
            : rect.int_x;

        int p_comp = found_x()
            ? m_game.current_pos.y
            : m_game.current_pos.x;

        int& p_other_comp = found_x()
            ? ret.x
            : ret.y = m_game.current_pos.y;

        (found_x() ? ret.y : ret.x) = search(interval, p_comp);

        /* Cache the current position before the game gets updated */
        save_current_pos();

        assert(0 <= ret.x < m_game.building.width
               && 0 <= ret.y < m_game.building.height
               && "Agent is returning an invalid move");

        return ret;
    }

    void update_data(Heat move_heat) {
        assert(prev_pos != m_game.current_pos
               && "Called update_data without having played a move");

        last_heat = move_heat;

        Interval& interval = found_x() ? rect.int_y : rect.int_x;
        int prev = found_x() ? prev_pos.y : prev_pos.x;
        int curr = found_x() ? m_game.current_pos.y : m_game.current_pos.x;
        int midpoint = (prev + curr) / 2;

        /*
         * If the heat is neutral, the current interval is shrinked to a point
         * and we record the value just found.
         */
        if (last_heat == Heat::neutral) {
            (found_x() ? bomb_y : bomb_x) = interval.m = interval.M = midpoint;
        }

        /* Otherwise, shrink the interval according to move_heat and the
         * direction of the previous move.
         */
        else {
            int diff = curr - prev;

            if (last_heat == Heat::warm) {
                if (interval.size() < 4 && (curr == interval.m || curr == interval.M)) {
                    (found_x() ? bomb_y : bomb_x) = interval.m = interval.M = curr;
                }
                if (prev < curr) {
                    interval.m = midpoint;
                }
                else {
                    interval.M = midpoint;
                }
            }
            else {
                if (interval.size() < 4 && (curr == interval.m || curr == interval.M)) {
                    (found_x() ? bomb_y : bomb_x) = (curr == interval.m ? interval.M : interval.m);
                }
                if (prev < curr) {
                    interval.M = midpoint;
                }
                else {
                    interval.m = midpoint;
                }
            }
            if (interval.size() == 1) {
                (found_x() ? bomb_y : bomb_x) = (curr == interval.m ? interval.M : interval.m);
            }
        }
    }

private:
    const Game& m_game;

    Rectangle rect;

    Window prev_pos;
    Heat last_heat;

    int bomb_x{ -1 };
    int bomb_y{ -1 };

    bool found_x() const { return bomb_x > -1; }
    bool found_y() const { return bomb_y > -1; }

    /**
     * Perform one iteration of a 1-dimensional binary search.
     *
     * @Note  If pos is in the middle of an even-length interval, opposite
     * returns pos itself.
     */
    int search(const Interval& interval, int p) const {
        int Max = found_x() ? m_game.building.height - 1
            : m_game.building.width - 1;

        int q = opposite(interval.m, interval.M, Max, p);

        if (q == m_game.current_pos.x) {
            q = q < Max - 2 ? q + 2 : q - 2;
        }

        return q;
    }

    void save_current_pos() {
        prev_pos = m_game.current_pos;
    }
};

}  // namespace sok2

#endif // AGENT_H_
