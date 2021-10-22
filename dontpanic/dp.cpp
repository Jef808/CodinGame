#include "dp.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

namespace dp {

/// Globals
namespace {

    GameParams params {};
    State root_state {};

} // namespace

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

    for (int i = 0; i < n_elevators; ++i) {
        auto& el = params.elevators.emplace_back();
        _in >> el.floor >> el.pos;
    }

    // First turn because not everything is initialized
    char d;
    _in >> ps->floor >> ps->pos >> d;
    _in.ignore();

    ps->dir = (d == 'L' ? State::Left : State::Right);
    ps->turn = 1;

    params.entry_pos = ps->pos;

    // Order the elevators by floor then by pos
    std::sort(params.elevators.begin(), params.elevators.end(), [](const auto& a, const auto& b) {
        return a.floor < b.floor
            || (a.floor == b.floor && a.pos < b.pos);
    });
}

const GameParams* Game::get_params() const
{
    return &params;
}

#if EXTRACTING_ONLINE_DATA
#include <sstream>

void extract_online_init()
{
    Game game;
    game.init(std::cin);

    std::stringstream ss {};

    ss << params.height << ' '
       << params.width << ' '
       << params.max_round << ' '
       << params.exit_floor << ' '
       << params.exit_pos << ' '
       << params.max_clones << ' '
       << params.n_add_elevators << ' '
       << params.elevators.size()
       << '\n';

    for (const auto& el : params.elevators) {
        ss << el.floor << ' '
           << el.pos
           << '\n';
    }

    char d;
    ss << ps->floor << ' '
       << ps->pos << ' '
       << d;
    << (d == 'L' ? State::Left : State::Right)
    << '\n';

    out << ss.str() << std::endl;
    out << "Successfully read all data." << std::endl;
}

#endif // EXTRACTING_ONLINE_DATA

} // namespace dp
