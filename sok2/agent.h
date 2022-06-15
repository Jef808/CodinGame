#ifndef AGENT_H_
#define AGENT_H_

#include "types.h"
#include "io.h"
#include "utils.h"

#include <cassert>

namespace sok2 {

class Agent {
public:
    Agent(const Game& game)
        : m_game{ game }
        , min_x{ 0 }
        , max_x{ game.building.width }
        , min_y{ 0 }
        , max_y{ game.building.height }
        , prev_pos{ game.current_pos }
    {}

    Window choose_move() const {
        if (not found_x) {
            return search_x();
        }
        if (not found_y) {
            return search_y();
        }
        return get_bomb();
    }

    void update_data(Heat move_heat) {
        if (not found_x) {
            if (move_heat == Heat::neutral) {
                found_x = true;
                min_x = max_x = (prev_pos.x + m_game.current_pos.x) / 2;
            }
            else {
                int diff = m_game.current_pos.x - prev_pos.x;
                int& bound = diff > 0 ? move_heat == Heat::warm ? max_x : min_x
                    : move_heat == Heat::warm ? min_x : max_x;
                bound = (prev_pos.x + m_game.current_pos.x) / 2;
            }
        }
        else {
            if (move_heat == Heat::neutral) {
                found_y = true;
                min_y = max_y = (prev_pos.y + m_game.current_pos.y) / 2;
            }
            else {
                int diff = m_game.current_pos.y - prev_pos.y;
                int& bound = diff > 0 ? move_heat == Heat::warm ? max_y : min_y
                    : move_heat == Heat::warm ? min_y : max_y;
                bound = (prev_pos.y + m_game.current_pos.y) / 2;
            }
        }
        prev_pos = m_game.current_pos;
    }

private:
    const Game& m_game;

    bool found_x{ false };
    bool found_y{ false };

    int min_x;
    int max_x;
    int min_y;
    int max_y;

    Window prev_pos;

    Window search_x() const {
        int nx = opposite(min_x, max_x, m_game.current_pos.x);
        if (nx == m_game.current_pos.x) {
            nx = nx < max_x - 2 ? nx + 2 : nx - 2;
        }
        return { nx, m_game.current_pos.y };
    }

    Window search_y() const {
        int ny = opposite(min_y, max_y, m_game.current_pos.y);
        if (ny == m_game.current_pos.y) {
            ny = ny < max_x - 2 ? ny + 2 : ny - 2;
        }
        return { m_game.current_pos.x, ny };
    }

    Window get_bomb() const {
        assert(found_x && found_y && "Trying to get bomb before grid is solved");
        assert(min_x == max_x && min_y == max_y && "Solved grid but bounds are not equal");
        return { min_x, min_y };
    }
};

}  // namespace sok2

#endif // AGENT_H_
