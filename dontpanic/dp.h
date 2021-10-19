#ifndef DP_H_
#define DP_H_

#include "types.h"

#include <array>
#include <iosfwd>
#include <string>
#include <vector>

namespace dp {

struct Elevator {
    int floor;
    int pos;
};

struct State {
    enum Dir { Left, Right } dir;
    int floor;
    int pos;
    int turn;
    int used_clones;
    std::array<Elevator, max_height> new_elevators;
};

struct GameParams {
    int height;
    int width;
    int max_round;
    int exit_floor;
    int exit_pos;
    int max_clones;
    int n_add_elevators;
    int entry_pos;
    std::vector<Elevator> elevators;
};

class Game {
public:
    Game() = default;
    void init(std::istream&);
    void view() const;
    const State* state() const;
    const GameParams* get_params() const;
private:
    State* ps;
};

inline const State* Game::state() const { return ps; }

} // namespace dp

extern void extract_online_init(std::ostream&);

// TODO Is this needed if I want to allow the agent access?
extern dp::GameParams params;


#endif // DP_H_
