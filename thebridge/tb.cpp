#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "tb.h"

namespace tb {

/// Globals
namespace {

    tb::Params params{ };
    tb::State root_state{ };

    std::array<std::vector<int>, 4> nex_holes{ };
    int last_hole = 0;

    /// Used to populate the nex_holes data
    void nex_holes_dists();

    void input_turn(std::istream& _in, tb::State& st);
}

const State* Game::state() const { return &root_state; }
const Params* Game::parameters() const { return &params; }

void Game::init(std::istream& _in)
{
    std::string buf;

    _in >> params.start_bikes
        >> params.min_bikes;
    _in.ignore();

    for (int i = 0; i < 4; ++i) {
        std::getline(_in, buf);
        std::transform(buf.begin(), buf.end(), std::back_inserter(params.road[i]),
            [](const auto c) { return c == '0' ? Cell::Hole : Cell::Bridge; });

        // Extend the road for safety.
        for (int j = 0; j < 10; ++j) {
            params.road[i].push_back(Cell::Bridge);
        }
    }

    input_turn(_in, root_state);

    nex_holes_dists();
}

namespace {

    /**
 * Compute and store the distance from each position to its next hole.
 * This is needed for the move simulations.
 */
    void nex_holes_dists()
    {
        last_hole = 0;
        for (int i = 0; i < 4; ++i) {
            auto holed_it = std::back_inserter(nex_holes[i]);
            auto pos_it = params.road[i].begin();
            while (pos_it != params.road[i].end()) {
                auto holes_beg = std::find(pos_it, params.road[i].end(), Cell::Hole);
                auto holes_end = std::find(holes_beg, params.road[i].end(), Cell::Bridge);
                auto dist = std::distance(pos_it, holes_beg);
                if (holes_beg == params.road[i].end()) {
                    std::fill_n(holed_it, dist, Max_length);
                }
                else {
                    std::generate_n(holed_it, dist, [n=dist]() mutable {return n--;});
                    std::fill_n(holed_it, std::distance(holes_beg, holes_end), 0);
                }
                pos_it = holes_end;
            }
        }
    }

    void input_turn(std::istream& _in, State& st)
    {
        int x, y, a; // NOTE: x-coord, y-coord, active-or-not
        _in >> st.speed;
        _in.ignore();

        for (int i = 0; i < params.start_bikes; ++i) {
            _in >> x >> y >> a;
            _in.ignore();
            st.bikes[y] = (a == 1);
        }
        st.pos = x;
    }

    inline void wait(State& s)
    {
        for (int i = 0; i < 4; ++i)
            s.bikes[i] &= nex_holes[i][s.pos] > s.speed;
        s.pos += s.speed;
    }

    inline void slow(State& s)
    {
        --s.speed;
        wait(s);
    }

    inline void speed_up(State& s)
    {
        ++s.speed;
        wait(s);
    }

    inline void jump(State& s)
    {
        for (int i = 0; i < 4; ++i) {
            s.bikes[i] &= params.road[i][s.pos + s.speed] != Cell::Hole;
        }
        s.pos += s.speed;
    }

    inline void up(State& s)
    {
        if (s.bikes[0])
            return;

        for (int i = 0; i < 3; ++i) {
            s.bikes[i] = s.bikes[i + 1]
                && nex_holes[i + 1][s.pos] >= s.speed
                && nex_holes[i][s.pos] > s.speed;
        }
        s.bikes[3] = false;
        s.pos += s.speed;
    }

    inline void down(State& s)
    {
        if (s.bikes[3])
            return;

        for (int i = 3; i > 0; --i) {
            s.bikes[i] = s.bikes[i - 1]
                && nex_holes[i - 1][s.pos] >= s.speed
                && nex_holes[i][s.pos] > s.speed;
        }
        s.bikes[0] = false;
        s.pos += s.speed;
    }

} // namespace

void Game::apply(const State& state, Action a, State& s) const
{
    s = state;
    switch (a) {
    // case Action::Wait:
    //     wait(s);
    //     break;
    case Action::Slow:
        slow(s);
        break;
    case Action::Speed:
        speed_up(s);
        break;
    case Action::Jump:
        jump(s);
        break;
    case Action::Up:
        up(s);
        break;
    case Action::Down:
        down(s);
        break;
    default: {
        if (a == Action::None)
            std::cerr << "Game: trying to apply Action::None"
                << std::endl;
        assert(false);
    }

    }

    ++s.turn;
    if (s.pos >= params.road[0].size())
        s.pos = params.road[0].size() - 1;
}

int n_conseq_holes()
{
    int res = 0;
    for (int i = 0; i < 4; ++i) {
        auto it = params.road[i].begin();
        while (it != params.road[i].end())
        {
            it = std::find(it, params.road[i].end(), Cell::Hole);
            auto n_conseq = std::distance(it, std::find(it, params.road[i].end(), Cell::Bridge));
            res = n_conseq > res ? n_conseq : res;
            it += n_conseq;
        }
    }
    return res;
}

const std::vector<Action>& Game::valid_actions(const State& s) const
{
    static const int longest_jump = n_conseq_holes();
    static std::array<Action, 5> actions = {
        Action::Speed, Action::Jump, Action::Up, Action::Down, Action::Slow
    };
    static std::vector<Action> ret;
    ret.clear();
    auto out = std::back_inserter(ret);

    // Speed is our first choice, but we cap the speed
    // to reduce number of candidates
    if (s.speed < 2 * longest_jump)
        out = Action::Speed;

    if (s.speed == 0) {
        return ret;
    }

    out = Action::Jump;

    // Cannot go up if 0 is occupied
    if (!s.bikes[0])
        out = Action::Up;

    // Cannot go down if 3 is occupied
    if (!s.bikes[3])
        out = Action::Down;

    // Don't stop the bikes
    if (s.speed > 1)
        out = Action::Slow;

    return ret;
}

int Game::find_last_hole() const
{
    int _ret = 0;
    for (int i=0; i<4; ++i) {
        auto last_hole = std::distance(nex_holes[i].begin(),
                                       std::find(nex_holes[i].begin(), nex_holes[i].end(), Max_length));
        _ret = last_hole > _ret ? last_hole : _ret;
    }
    return _ret;

}

void Game::show(std::ostream& _out, const State& s) const
{
    const auto& road = params.road;
    const auto& bikes = s.bikes;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < s.pos; ++j)
            _out << (road[i][j] == Cell::Hole ? '0' : '-');
        _out << (s.bikes[i] ? 'B' : (road[i][s.pos] == Cell::Hole ? '0' : '-'));
        for (int j = s.pos + 1; j < road[i].size(); ++j)
            _out << (road[i][j] == Cell::Hole ? '0' : '-');
        _out << '\n';
    }
    _out << std::endl;
}

} // namespace tb
