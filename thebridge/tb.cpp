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

    tb::Params params;
    tb::State root_state;

    std::array<std::vector<int>, 4> nex_holes;
    int last_hole;

    /// Used to populate the nex_holes data
    void nex_holes_dists();

    void input_turn(std::istream& _in, tb::State& st);
}

namespace Zobrist {

    std::array<uint32_t, 16> key_bikes {};
    std::array<uint32_t, 500> key_pos {};
    std::array<uint32_t, 50> key_speed {};
    tb::Key get_key_bikes(const tb::State& st)
    {
        size_t lanes = 0;
        for (int i = 0; i < 4; ++i) {
            lanes += st.bikes[i] * (2 << i);
        }
        return key_bikes[lanes];
    }
    tb::Key get(const tb::State& st)
    {
        return get_key_bikes(st) ^ key_pos[st.pos] ^ key_speed[st.speed];
    }

} // namespace tb::Zobrist

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

    std::random_device rd;
    std::mt19937 eng { rd() };
    std::uniform_int_distribution<uint32_t> dist(1, std::numeric_limits<uint32_t>::max());
    for (int i = 0; i < 16; ++i)
        Zobrist::key_bikes[i] = dist(eng);
    for (int i = 0; i < 500; ++i)
        Zobrist::key_pos[i] = dist(eng);
    for (int i = 0; i < 50; ++i)
        Zobrist::key_speed[i] = dist(eng);
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
            auto holed_i = std::back_inserter(nex_holes[i]);
            auto pos_it = params.road[i].begin();
            auto hol_it = pos_it;
            while (true) {
                hol_it = std::find(pos_it + 1, params.road[i].end(), Cell::Hole);
                auto dist = std::distance(pos_it, hol_it);
                if (hol_it == params.road[i].end()) {
                    std::fill_n(holed_i, dist, Max_length);
                    break;
                } else {
                    pos_it += dist;
                    last_hole += dist;
                    for (; dist >= 0; --dist)
                        holed_i = dist;
                }
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
            st.bikes[i] = (a == 1);
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
        s.key ^= (Zobrist::key_speed[s.speed] ^ Zobrist::key_speed[s.speed - 1]);
        --s.speed;
        wait(s);
    }

    inline void speed_up(State& s)
    {
        s.key ^= (Zobrist::key_speed[s.speed] ^ Zobrist::key_speed[s.speed + 1]);
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

State& Game::apply(State& s, Action a) const
{
    s.key ^= Zobrist::key_pos[s.pos];
    s.key ^= Zobrist::get_key_bikes(s);
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
    default:
        assert(false);
    }

    ++s.turn;

    s.key ^= Zobrist::key_pos[s.pos];
    s.key ^= Zobrist::get_key_bikes(s);

    return s;
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

const std::array<Action, 5>& Game::valid_actions(const State& s) const
{
    static const int longest_jump = n_conseq_holes();
    static std::array<Action, 5> actions = {
        Action::Speed, Action::Jump, Action::Up, Action::Down, Action::Slow
    };

    // Speed is our first choice, but we cap the speed
    // to reduce number of candidates
    if (s.speed >= longest_jump + 1)
        actions[0] = Action::None;

    if (s.speed == 0) {
        actions[1] = actions[2] = actions[3] = actions[4] = Action::None;
        return actions;
    }

    // Always include Jump

    // Cannot go up if 0 is occupied
    if (s.bikes[0])
        actions[2] = Action::None;
    // Cannot go down if 3 is occupied
    if (s.bikes[3])
        actions[3] = Action::None;

    // Don't stop the bikes
    if (s.speed <= 1)
        actions[4] = Action::None;

    return actions;
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
        _out << std::endl;
    }
}

} // namespace tb
