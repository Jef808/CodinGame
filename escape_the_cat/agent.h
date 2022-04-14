#ifndef AGENT_H_
#define AGENT_H_


#include "utilities.h"
#include "escape_cat.h"

namespace escape_cat {

class Agent {
public:
    Agent() = default;

    Point choose_move(const Game& game, std::string& debug);

    /**
     * The region of the boundary circle where the cat would
     * catch the mouse if it were to reach it within the specified
     * number of turns
     */
    [[nodiscard]] CircleArc danger_zone(const State& state, int n_turns) const;


    void output_choice(std::ostream& _out, const Point& p, const State& state);

private:
    static inline Radian m_arc_padding { -1.0 };
};

} // namespace



#endif // AGENT_H_
