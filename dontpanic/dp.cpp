#include "types.h"
#include "dp.h"

#if FMT_ENABLED
  #include "viewutils.h"
#endif

#include <algorithm>
#include <cassert>
#include <sstream>
#include <string_view>
#include <vector>


namespace dp {

/// Globals
namespace {

    GameParams params{};
    State root_state{};

    std::string board_view{};
    std::string buf{};
    void init_board_view();

}  // namespace

void Game::init(std::istream& _in)
{
    ps = &root_state;
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

    // Order the elevators by floor then by pos
    std::sort(params.elevators.begin(), params.elevators.end(), [](const auto& a, const auto& b) {
        return a.floor < b.floor
            || (a.floor == b.floor && a.pos < b.pos);
    });

    // First turn because not everything is initialized
    char d;
    _in >> ps->floor >> ps->pos >> d;
    _in.ignore();

    ps->dir = (d == 'L' ? State::Left : State::Right);
    ps->turn = 1;

    params.entry_pos = ps->pos;
}

const GameParams* Game::get_params() const {
    return &params;
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

} // namespace dp
