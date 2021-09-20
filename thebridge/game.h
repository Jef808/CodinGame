#ifndef GAME_H_
#define GAME_H_

#include <array>
#include <iosfwd>
#include <vector>


namespace CGbridge {


/**
 * The road containing the holes our bikes need to avoid.
 */
enum class Cell { Bridge, Hole };
typedef std::array<std::vector<Cell>, 4> Road;

/**
 * Parameters setting the goals to win
 * (set once at initialization).
 */
struct GameParams {
    size_t road_length;
    int min_bikes;
    int start_bikes;
};

/**
 * The possible actions at each turn.
 */
enum class Action { Speed, Jump, Up, Down, Slow, Wait };

/**
 * The mutable part of the game's data.
 */
struct State {
    size_t pos;
    size_t speed;
    std::array<bool, 4> active;
    int turn;
};

/**
 * Stores all information about the game rules and is used
 * to mimic navigating the "State/Space" tree by calling
 * apply() and undo().
 */
class Game {
public:
    static void init(std::istream&);
    Game() = default;

    apply(Action a);
    undo();

private:
    Road* p_road;
    GameParams* p_params;
    State* states;
};

extern std::ostream& operator<<(std::ostream& _out, const Game&);

} // namespace CGbridge

#endif // GAME_H_
