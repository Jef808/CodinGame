#ifndef __TYPES_H_
#define __TYPES_H_

#define FMT_ENABLED 0
#define RUNNING_OFFLINE FMT_ENABLED
#define EXTRACTING_ONLINE_DATA 0

#endif

#ifndef DP_H_
#define DP_H_

#include <iosfwd>
#include <string>

namespace dp {

enum class Cell;
struct State;

class Game {
public:
    Game() = default;
    void init(std::istream&);
    void view() const;
    const State* state() const;
private:
    State* ps;
};

inline const State* Game::state() const { return ps; }

} // namespace dp

extern void extract_online_init(std::ostream&);



#endif // DP_H_

#ifndef AGENT_H_
#define AGENT_H_

namespace dp {

class Agent {
public:
    explicit Agent(Game& game);
    std::string best_choice();
private:
    Game& game;
};

inline std::string Agent::best_choice() {
    auto ret = "WAIT";
    return ret;
}


} // namespace dp

#endif // AGENT_H_

#if FMT_ENABLED
  #include "viewutils.h"
#endif

#include <algorithm>
#include <cassert>
#include <sstream>
#include <string_view>
#include <vector>


namespace dp {

constexpr int max_turns = 200;
constexpr int max_height = 15;
constexpr int max_width = 100;
constexpr int max_n_clones = 50;
constexpr int max_n_elevators = 100;
constexpr int max_time_ms = 100;

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
} params{};

struct State {
    enum Dir { Left, Right } dir;
    int floor;
    int pos;
    int turn;
    std::vector<Elevator> new_elevators;
};

/// Globals
namespace {

    State states[max_turns + 1];

    std::string board_view{};
    std::string buf{};
    void init_board_view();

}  // namespace

void Game::init(std::istream& _in)
{
    std::fill_n(&states[0], max_turns+1, State{});
    ps = &states[0];

    params.entry_pos = -1;
    int n_elevators;
    _in >> params.height
        >> params.width
        >> params.max_round
        >> params.exit_floor
        >> params.exit_pos
        >> params.max_clones
        >> params.n_add_elevators
        >> n_elevators;

    for (int i=0; i<n_elevators; ++i) {
        auto& el = params.elevators.emplace_back();
        _in >> el.floor >> el.pos;
    }

    // First turn because not everything is initialized
    std::string buf;
    _in >> ps->floor >> ps->pos >> buf;
    _in.ignore();

    ps->dir = (buf[0] == 'L' ? State::Left : State::Right);
    ps->turn = 1;

    params.entry_pos = ps->pos;
}


#if FMT_ENABLED

void Game::view() const
{
    Square sq{};

    board_view.reserve(params.height * 4 * (params.width + 3));
    //buf.reserve(params.height * 4 * (params.width + 3));
    auto out = std::back_inserter(board_view);

    for (int i=0; i<params.height; ++i) {
        fmt::format_to(out, "{:w}", sq);

        const int floor = params.height - i - 1;

        auto next_el = std::find_if(
            params.elevators.begin(), params.elevators.end(),
            [floor](const auto& el){
                return el.floor == floor;
        });

        bool past_last_elevator = next_el == params.elevators.end();
        const bool is_exit_floor = params.exit_floor == floor;
        const bool is_entry_floor = floor == 0;
        const bool is_current_floor = floor == ps->floor;

        for (int j=0; j<params.width; ++j) {
            if (is_exit_floor && j == params.exit_pos) {
                /// Exit
                fmt::format_to(out, "{:o}", sq);
                fmt::format_to(out, "{:u}", sq);
            }
            else if (is_entry_floor && j == params.entry_pos) {
                /// Entry
                fmt::format_to(out, "{:i}", sq);
                fmt::format_to(out, "{:n}", sq);
            }
            else if (!past_last_elevator && next_el->pos == j) {
                /// Elevator
                /// TODO: 'new' elevators with different color
                fmt::format_to(out, "{:e}", sq);
                fmt::format_to(out, "{:e}", sq);

                next_el = std::find_if(
                    next_el+1, params.elevators.end(),
                    [floor](const auto& el){
                        return el.floor == floor;
                });

                past_last_elevator = next_el == params.elevators.end();
            }
            else if (is_current_floor && ps->pos == j) {
                /// Current leading clone
                /// TODO: Indicate direction, etc...
                fmt::format_to(out, "{:c}", sq);
                fmt::format_to(out, "{:c}", sq);
            }
            else {
                /// Default case
                fmt::format_to(out, "{:s}", sq);
                fmt::format_to(out, "{:s}", sq);
            }
        }
        /// Wall
        fmt::format_to(out, "{:w}\n", sq);
    }
    fmt::print("{}", board_view);
}

#endif // FMT_ENABLED


#if EXTRACTING_ONLINE_DATA
#include <stringstream>

void extract_online_init(std::ostream& out)
{
    Game game;
    game.init(std::cin);

    std::stringstream ss{};

    ss  << params.height << ' '
        << params.width << ' '
        << params.max_round << ' '
        << params.exit_floor << ' '
        << params.exit_pos << ' '
        << params.max_clones << ' '
        << params.n_add_elevators << ' '
        << params.elevators.size() << '\n';

    for (const auto& el : params.elevators) {
        ss  << el.floor << ' '
            << el.pos << '\n';
    }

    out << ss.str() << std::endl;
    out << "Successfully read all data." << std::endl;
}

#endif // EXTRACTING_ONLINE_DATA


Agent::Agent(Game& _game) :
    game(_game)
{
}

}  // namespace dp



/// MAIN
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

namespace {


void ignore_turn(std::istream& _in)
{
    static std::string _turn_buffer;
    std::getline(_in, _turn_buffer);
}

void solve(dp::Game& game)
{
    dp::Agent agent(game);

    while (true) {
        std::cout << agent.best_choice() << std::endl;
        ignore_turn(std::cin);
    }
}


}  // namespace


using namespace dp;

int main(int argc, char *argv[]) {

#if RUNNING_OFFLINE
    if (argc < 2)
    {
        fmt::print(stderr, "USAGE: {} [Test number]\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string fn;
    fmt::format_to(std::back_inserter(fn), "../data/test{}.txt", argv[1]);

    std::ifstream ifs{ fn };

    if (!ifs) {
        ifs.clear();
        fn = fn.substr(3);
        ifs.open(fn, std::ios_base::in);
        if (!ifs) {
            fmt::print("Failed to open both {} and ../{}\n", fn, fn);
            return EXIT_FAILURE;
        }
    }

    Game game;
    game.init(ifs);

    game.view();
    fmt::print("{}\n", "Successfully initialized the view");

    fmt::print("{}\n", "Successfully initialized the agent");
    fmt::format("{}\n", "Starting main loop");

/// Running Online
#else

    std::ios_base::sync_with_stdio(false);

  #if EXTRACTING_ONLINE_DATA
    extract_online_init(std::ostream&);
    return EXIT_SUCCESS;

  #else
    Game game;
    game.init(std::cin);

  #endif
#endif

    /// Main loop
    Agent agent{game};

    solve(game);


    std::cerr << "Exiting the program."
        << std::endl;


    return EXIT_SUCCESS;
}
