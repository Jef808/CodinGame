#ifndef DP_H_
#define DP_H_

#include "types.h"

#include <array>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace dp {

struct Elevator {
    int floor;
    int pos;
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

struct State {
    enum Dir { Left, Right } dir;
    int floor;
    int pos;
    int turn;
    int used_clones;
    int used_elevators;
    State* prev;
};

enum class Action {
    Wait, Block, Elevator
};

class Game {
public:
    Game() = default;
    void init(std::istream&);
    const State* state() const;
    const GameParams* get_params() const;
    const State& get_root_state() const;
private:
    State* ps;
};

inline const State* Game::state() const { return ps; }

} // namespace dp

extern void extract_online_init();


#endif // DP_H_
