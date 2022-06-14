#ifndef FAKEJUDGE_H_
#define FAKEJUDGE_H_

#include "types.h"
#include "utils.h"
#include "io.h"

#include <cassert>
#include <string>


namespace sok2::fakejudge {

    const std::string colder_s {"COLDER"};
    const std::string warmer_s {"WARMER"};
    const std::string same_s {"SAME"};


class FakeJudge {
public:
    FakeJudge(Game& game, const Window& bomb)
        : m_game{ game } , m_bomb{ bomb }
    {}

    /**
     * Read a player output from @is and output the judge's next input to @os.
     */
    void process_player_output(std::istream& is, std::ostream& os);

    const Game& game() const { return m_game; }
    const Window& bomb() const { return m_bomb; }

private:
    Game& m_game;
    const Window m_bomb;

    /**
     * Get the heat value if player jumps to @next.
     */
    heat get_heat(const Window& next) const;

    /**
     * Get the string to output given the heat value @h.
     */
    const std::string& get_output(heat h) const;
};


inline void FakeJudge::process_player_output(std::istream &is, std::ostream &os) {
    Window next;
    is >> next;
    os << get_output(get_heat(next));
    m_game.current_pos = next;
}


inline heat FakeJudge::get_heat(const Window& next) const {
    double current_dist = distance(m_bomb, m_game.current_pos);
    double next_dist = distance(m_bomb, next);

    return current_dist < next_dist ? heat::colder
        : current_dist > next_dist ? heat::warmer
        : heat::same;
}

inline const std::string& FakeJudge::get_output(heat h) const {
    switch (h) {
        case heat::colder: return colder_s;
        case heat::warmer: return warmer_s;
        case heat::same:   return same_s;
        default: assert(false && "Invalid heat input in FakeJudge's get_output");
    }
}

}  // namespace sok2::fakejudge

#endif // FAKEJUDGE_H_
