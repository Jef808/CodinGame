#ifndef TB_H_
#define TB_H_

#include <array>
#include <cassert>
#include <iosfwd>
#include <vector>

#include "types.h"


namespace tb {

/**
 * The mutable part of the game.
 */
struct State {
    Key key;
    size_t pos;
    size_t speed;
    std::array<bool, 4> bikes{ 0, 0, 0, 0 };
    int turn{ 0 };
    State* prev;
};
typedef std::array<std::vector<Cell>, 4> Road;

/**
 * The immutable part of the game.
 */
struct Params {
    Road road;
    int start_bikes;
    int min_bikes;
};

extern Params params;

class Agent;

/**
 * Class implementing the game simulation.
 */
class Game {
public:
    enum class Status { Loss=0, Unknown=1, Win };
    using ActionList = std::vector<Action>;

    static void init(std::istream&);
    Game() = default;
    void set(State&);
    void set(State&, Agent*);
    void apply(const Action a, State& st);
    void undo();
    bool is_won() const;
    bool is_lost() const;
    uint32_t key() const;
    int turn() const { return pstate->turn; }
    int pos() const { return pstate->pos; }
    size_t get_speed() const { return pstate->speed; }
    size_t road_length() const { return pparams->road[0].size(); }
    int n_bikes() const;
    size_t n_bikes_start() const { return pparams->start_bikes; }
    int ratio_bikes_left() const;
    //size_t hole_dist(size_t lane, size_t pos) const;
    //bool win_past_all_holes() const;
    Params const* parameters() const { return pparams; }
    Agent* handling_agent() const { return phandling_agent; }
    std::array<Action, 5> candidates1() const;
    const std::vector<Action>& candidates() const;

private:
    State* pstate;
    Params * const pparams = &tb::params;
    Agent* phandling_agent;
    void apply(const Action a);
};

inline bool Game::is_won() const {
    return pstate->pos >= pparams->road[0].size() && !(is_lost());
}

inline Key Game::key() const {
    return pstate->key;
}

extern std::ostream& operator<<(std::ostream& out, const Action a);

} // namespace tb



#endif // TB_H_
