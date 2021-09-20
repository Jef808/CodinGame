#include "game.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <vector>

namespace CGbridge {

{
    GameParams params;
    Road road;
}


void input(std::istream& _in) {
    int n_bikes_start, n_bikes_min, road_length;
    std::string buf;

    _in >> params.start_bikes
        >> params.min_bikes; _in.ignore();

    static Road _road =
        [&_in]{
            Road _road{};
            for (int i=0; i<4; ++i)
            {

                std::getline(_in, buf);
                std::transform(buf.begin(), buf.end(), std::back_inserter(_road[i]),
                               [](const auto c){ return c == '0' ? Cell::Hole : Cell::Bridge; });
            }
            return _road;
        }();

    static const GameParams _params { road_length, n_bikes_min, n_bikes_start };

}

void input_turn(std::istream& _in, State& state)
{
    // Input the initial State
    std::stringstream ss{};
    int s, x, y, a;  // x-coord, y-coord, active-or-not
    _in >> state.speed; _in.ignore();

    for (int i=0; i<params.start_bikes; ++i)
    {
        _in >> x >> y >> a; _in.ignore();

        ss << x << ' ' << y << ' ' << a << '\n';
        state.active[y] = (a == 1);
    }
    state.pos = x;
}

void Game::init(std::istream& _in)
{
    input(_in, road, params);
}

std::ostream& operator<<(std::ostream& _out, const Cell);
std::ostream& operator<<(std::ostream& _out, const Game&);





} // namespace CGbridge
