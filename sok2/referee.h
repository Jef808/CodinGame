#ifndef REFEREE_H_
#define REFEREE_H_

#include "types.h"
#include "utils.h"
#include "io.h"

#include <cassert>
#include <string>

namespace sok2 {


const std::string colder_s  {"COLDER"};
const std::string warmer_s  {"WARMER"};
const std::string neutral_s {"SAME"};


class Referee {
public:
    Referee(Game& game, const Window& bomb)
        : m_game{ game } , m_bomb{ bomb }
    {}

    /**
     * Read a player output from @is and output the judge's next input to @os.
     *
     * @Return false if game is over, true otherwise.
     */
    bool process_turn(std::istream& is, std::ostream& os);

    /**
     * Get the heat value if player jumps to @next.
     */
    Heat get_heat(const Window& prev, const Window& next) const;

    const Game& game() const { return m_game; }
    const Window& bomb() const { return m_bomb; }

private:
    Game& m_game;
    const Window m_bomb;

    /**
     * Get the string to output given the heat value @h.
     */
    const std::string& to_string(Heat h) const;
};


inline bool Referee::process_turn(std::istream &is, std::ostream &os) {
    Window next;
    is >> next;
    os << to_string(get_heat(m_game.current_pos, next));
    m_game.current_pos = next;
    --m_game.turns_left;
    if (m_game.turns_left == 0 || m_game.current_pos == m_bomb) {
        return false;
    }
    return true;
}


inline Heat Referee::get_heat(const Window& prev, const Window& next) const {
    return compare_distances(m_bomb, next, prev);
}

inline const std::string& Referee::to_string(Heat h) const {
    switch (h) {
        case Heat::cold:    return colder_s;
        case Heat::warm:    return warmer_s;
        case Heat::neutral: return neutral_s;
        default: assert(false && "Invalid heat input in Referee's get_output");
    }
}

}  // namespace sok2

#endif // REFEREE_H_
