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
    int turn;
    State* prev;
};

/**
 * The immutable part of the game.
 */
struct Params {
    enum class Cell { Bridge, Hole };
    typedef std::array<std::vector<Cell>, 4> Road;
    Road road;
    int start_bikes;
    int min_bikes;
};

extern Params params;

/**
 * Class implementing the game simulation.
 */
class Game {
public:
    enum class Status { Loss=0, Unknown=1, Win };
    using ActionList = std::vector<Action>;

    static void init(std::istream&);
    Game() = default;
    void set(State& st);
    void apply(const Action a, State& st);
    void undo();
    bool is_won() const;
    bool is_lost() const;
    uint32_t key() const;
    int turn() const { return pstate->turn; }
    int pos() const { return pstate->pos; }
    size_t hole_dist(size_t lane, size_t pos) const;
    bool win_past_all_holes() const;
    const std::vector<Action>& candidates() const;

private:
    State* pstate;
    Params* const pparams = &tb::params;
};

inline bool Game::is_won() const {
    return pstate->pos >= pparams->road[0].size();
}

inline Key Game::key() const {
    return pstate->key;
}

} // namespace tb

extern std::ostream& operator<<(std::ostream& out, const tb::Action a);



#endif // TB_H_
