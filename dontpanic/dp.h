#ifndef DP_H_
#define DP_H_

#include <iosfwd>
#include <optional>
#include <vector>

namespace dp {

constexpr int max_turns = 200;
constexpr int max_height = 15;
constexpr int max_width = 100;
constexpr int max_n_clones = 50;
constexpr int max_n_elevators = 100;
constexpr int max_time_ms = 100;

enum class cell_t {
    Empty,
    WallL,
    WallR,
    Elevator,
    Entry,
    Exit,
    Nb = 6
};

enum class Type { Elevator,
    Clone };
enum class Dir { Right,
    Left };
struct Entity {
    Type type;
    int pos;
    int floor;
    std::optional<Dir> dir;
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
    std::vector<Entity> elevators;
};

struct State {
    Dir dir;
    int floor;
    int pos;
    int turn;
    int clones;
    int player_elevators;
    State* prev;
};

enum class Action {
    None,
    Wait,
    Block,
    Elevator
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
