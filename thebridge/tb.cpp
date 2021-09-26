#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "tb.h"
#include "types.h"
#include "viewer.h"

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

inline void speed(State& s)
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

void Game::apply(const Action a, State& st)
{
    st = *pstate;
    st.prev = pstate;
    pstate = &st;
    ++pstate->turn;
    st.key ^= Zobrist::key_pos[pstate->pos];
    st.key ^= Zobrist::get_key_bikes(*pstate);
    switch (a) {
    case Action::Wait:
        wait(*pstate);
        break;
    case Action::Slow:
        slow(*pstate);
        break;
    case Action::Speed:
        speed(*pstate);
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
    st.key ^= Zobrist::key_pos[pstate->pos];
    st.key ^= Zobrist::get_key_bikes(*pstate);
}

void Game::undo()
{
    assert(pstate->prev != nullptr);
    pstate = pstate->prev;
}

int n_bikes(const State& st)
{
    return std::count(st.bikes.begin(), st.bikes.end(), true);
}

bool Game::is_lost() const
{
    return pstate->turn > 50 || n_bikes(*pstate) < params.min_bikes;
}

int Game::ratio_bikes_left() const
{
    return 500 * (n_bikes(*pstate) - pparams->min_bikes + 1) / (pparams->start_bikes - pparams->min_bikes + 1) - 250;
}

const std::vector<Action>& Game::candidates() const
{
    static std::vector<Action> cands;
    cands.clear();
    size_t p = pstate->pos;
    size_t s = pstate->speed;

    if (s == 0) {
        cands.push_back(Action::Speed);
        return cands;
    }

    auto is_cand = [](Action a) {
        return std::find(cands.begin(), cands.end(), a) != cands.end();
    };
    int max_deaths = n_bikes(*pstate) - params.min_bikes;
    // Speed
    int n_deaths = 0;
    if (pstate->speed < 50) {
        for (int i = 0; i < 4; ++i) {
            n_deaths = pstate->bikes[i] && (nex_holes[i][p] <= s + 1);
        }
        if (n_deaths <= max_deaths)
            cands.push_back(Action::Speed);
    }
    // Jump
    n_deaths = 0;
    for (int i = 0; i < 4; ++i) {
        n_deaths += pstate->bikes[i] && (params.road[i][p+s] == Cell::Hole);
    }
    if (n_deaths <= max_deaths)
        cands.push_back(Action::Jump);
    // Slow
    n_deaths = 0;
    if (s > 0) {
        for (int i = 0; i < 4; ++i) {
            n_deaths += pstate->bikes[i] && (nex_holes[i][p] < s);
        }
        if (n_deaths <= max_deaths)
            cands.push_back(Action::Slow);
    }
    // Up
    n_deaths = 0;
    if (!pstate->bikes[0]) {
        for (int i = 1; i < 4; ++i) {
            n_deaths += (pstate->bikes[i] && (nex_holes[i][p] < s || nex_holes[i - 1][p] <= s));
        }
        if (n_deaths <= max_deaths)
            cands.push_back(Action::Up);
    }
    // Down
    n_deaths = 0;
    if (!pstate->bikes[3]) {
        for (int i = 0; i < 3; ++i) {
            n_deaths += pstate->bikes[i] && (nex_holes[i][p] < s || nex_holes[i + 1][p] <= s);
        }
        if (n_deaths <= max_deaths)
            cands.push_back(Action::Down);
    }
    // Mix up Up / Down
    if (is_cand(Action::Up) && is_cand(Action::Down) && rand() % 2)
        std::swap(cands[cands.size()-1], cands[cands.size()-2]);
    return cands;
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

void Game::view(std::ostream& out, const std::vector<Action>& actions) {
    //viewer::ExtRoad<Road, State> xroad;
    viewer::Viewer<Road, State, Game> v( params.road, *pstate, *this );

    v.view(out, actions.begin(), actions.end());
}


} // namespace tb
