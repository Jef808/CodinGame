#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "tb.h"
#include "types.h"

namespace tb {

Params params;

namespace {

  std::array<std::vector<int>, 4> nex_holes;
 /**
 * Compute and store the distance from each position to its next hole.
 * This is needed for the move simulations.
 */
    void nex_holes_dists()
    {
        for (int i = 0; i < 4; ++i) {
            auto& holed_i = nex_holes[i];
            auto pos_it = params.road[i].begin();
            auto hol_it = pos_it;
            do {
                hol_it = std::find(pos_it + 1, params.road[i].end(), Cell::Hole);
                while (pos_it != hol_it) {
                    holed_i.push_back(std::distance(pos_it, hol_it));
                    ++pos_it;
                }

            } while (hol_it != params.road[i].end());
        }
    }

} // namespace

namespace Zobrist {

    std::array<uint32_t, 16> key_bikes {};
    std::array<uint32_t, 500> key_pos {};
    std::array<uint32_t, 50> key_speed {};
    Key get_key_bikes(const State& st)
    {
        size_t lanes = 0;
        for (int i = 0; i < 4; ++i) {
            lanes += st.bikes[i] * (2 << i);
        }
        return key_bikes[lanes];
    }
    Key get(const State& st)
    {
        return get_key_bikes(st) ^ key_pos[st.pos] ^ key_speed[st.speed];
    }
}

void Game::init(std::istream& _in)
{
    int n_bikes_start, n_bikes_min, road_length;
    std::string buf;

    _in >> params.start_bikes
        >> params.min_bikes;
    _in.ignore();

    for (int i = 0; i < 4; ++i) {
        std::getline(_in, buf);
        std::transform(buf.begin(), buf.end(), std::back_inserter(params.road[i]),
            [](const auto c) { return c == '0' ? Cell::Hole : Cell::Bridge; });
        // Extend the road for safety.
        for (int j = 0; j<10; ++j) {
            params.road[i].push_back(Cell::Bridge);
        }
    }

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

void Game::set(State& st)
{
    assert(pstate != &st);
    pstate = &st;
    Key key = Zobrist::get(*pstate);
    pstate->key = key;
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
        s.bikes[i] &= params.road[i][s.pos+s.speed] != Cell::Hole;
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

void Game::apply(State& s, Action a) {
        
    s = *pstate;
    s.prev = pstate;
    pstate = &s;

    pstate->key ^= Zobrist::key_pos[pstate->pos];
    pstate->key ^= Zobrist::get_key_bikes(*pstate);
    switch (a) {
    case Action::Wait:
        wait(*pstate);
        break;
    case Action::Slow:
        slow(*pstate);
        break;
    case Action::Speed:
        speed_up(*pstate);
        break;
    case Action::Jump:
        jump(*pstate);
        break;
    case Action::Up:
        up(*pstate);
        break;
    case Action::Down:
        down(*pstate);
        break;
    default:
        break;
    }

    ++pstate->turn;

    pstate->key ^= Zobrist::key_pos[pstate->pos];
    pstate->key ^= Zobrist::get_key_bikes(*pstate);
}

void Game::undo()
{
    assert(pstate->prev != nullptr);
    pstate = pstate->prev;
}

int Game::n_bikes() const
{
    return std::count(pstate->bikes.begin(), pstate->bikes.end(), true);
}

int n_conseq_holes() {
    int n = 0;
    for (int i=0; i<4; ++i)
    {
        auto h = std::find(params.road[i].begin(), params.road[i].end(), Cell::Hole);

        if (h == params.road[i].end())
            return n;

        auto j = std::find(h, params.road[i].end(), Cell::Bridge);
        if (j == params.road[i].end())
            return n;

        int m = std::distance(h, j);
        n = m > n ? m : n;
    }

    return n;
}

std::array<Action, 5> Game::candidates() const
{
    static const int longest_jump = n_conseq_holes();
    static std::array<Action, 5> actions {
        Action::Speed, Action::Jump, Action::Up, Action::Down, Action::Slow
    };

    std::array<Action, 5> ret;
    ret.fill(Action::None);
    auto it = ret.begin();

    // Speed is our first choice, but we cap the speed
    // to mix up the candidates a little
    if (pstate->speed < longest_jump + 1)
        *it++ = actions[0];

    if (pstate->speed == 0) {
        return ret;
    }

    // Jump
    *it++ = actions[1];

    // Filter Up/Downs
    if (!pstate->bikes[0])
        *it++ = actions[2];
    if (!pstate->bikes[3])
        *it++ = actions[3];

    // Slow
    if (pstate->speed > 1)
        *it++ = actions[4];

    return ret;
}

void Game::show(std::ostream& _out) const
{
    const auto& road = params.road;
    const auto& bikes = pstate->bikes;

    for (int i=0; i<4; ++i) {
        for (int j=0; j<pstate->pos; ++j)
            _out << (road[i][j] == Cell::Hole ? '0' : '-');
        _out << (pstate->bikes[i] ? 'B' : (road[i][pstate->pos] == Cell::Hole ? '0' : '-'));
        for (int j=pstate->pos+1; j<road[i].size(); ++j)
            _out << (road[i][j] == Cell::Hole ? '0' : '-');
        _out << std::endl;
    }
}

std::ostream& operator<<(std::ostream& out, const tb::Action a)
{
    using tb::Action;
    switch (a) {
    case Action::Wait:
        return out << "WAIT";
    case Action::Slow:
        return out << "SLOW";
    case Action::Speed:
        return out << "SPEED";
    case Action::Jump:
        return out << "JUMP";
    case Action::Up:
        return out << "UP";
    case Action::Down:
        return out << "DOWN";
    default:
        return out << "NONE";
    }
}

} // namespace tb
