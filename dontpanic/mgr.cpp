#include "dp.h"

#include <fmt/format.h>

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>
#include <vector>

namespace dp {

enum class cell_t {
    Empty,
    WallL,
    WallR,
    Elevator,
    Entry,
    Exit,
    Nb = 6
};

struct Clone {
    int pos;
    int floor;
    enum { Right,
        Left } dir;
};

Clone& reverse(Clone& clone)
{
    clone.dir = Clone::Right ? Clone::Left : Clone::Right;
    return clone;
}

int pos_front(const Clone& clone)
{
    return clone.dir == Clone::Right ? clone.pos + 1 : clone.pos - 1;
}

class DpMgr {
public:
    enum class status { Uninitialized,
        Initialized,
        Ongoing,
        Lost,
        Won,
        Error } status;

    void load(dp::Game& game)
    {
        prm = game.get_params();
        width = prm->width + 2;
        height = prm->height;

        m_grid.reserve(width * height);

        std::fill_n(std::back_inserter(m_grid), width * height, cell_t::Empty);

        // add the walls
        for (int i = 0; i < prm->height; ++i) {
            m_grid[i * width] = cell_t::WallL;
            m_grid[(i + 1) * width - 1] = cell_t::WallR;
        }

        // add the elevators
        for (const auto& el : prm->elevators) {
            m_grid[el.floor * width + (el.pos + 1)] = cell_t::Elevator;
        }

        // add the entry and exit
        m_grid[prm->entry_pos + 1] = cell_t::Entry;
        m_grid[prm->exit_pos + prm->exit_floor * width] = cell_t::Exit;

        status = status::Initialized;
    }

    bool pre_input()
    {
        ++n_turns;

        if (n_turns > prm->max_round) {
            status = status::Lost;
            return false;
        }

        if (clones_spawned < prm->max_clones) {
            if (spawn_cd == 0) {
                Clone spawn { prm->entry_pos, 0, Clone::Right };
                m_clones.push_back(spawn);
                ++clones_spawned;
                spawn_cd = 3;
            }

            --spawn_cd;
        }

        return true;
    }

    bool input(Action a, std::ostream& _err = std::cerr)
    {
        assert(n_turns <= prm->max_round);
        assert(clones_spawned <= prm->max_clones);
        assert(elevators_used <= prm->n_add_elevators);
        assert(!(m_clones.front().floor == prm->exit_floor && m_clones.front().pos == prm->exit_pos));

        if (a != Action::Wait && m_clones.empty()) {
            _err << "invalid action: no clones available"
                 << std::endl;

            status = status::Error;

            return false;
        }

        Clone clone = m_clones.front();
        cell_t cell = m_grid[(clone.pos + 1) + (clone.floor * width)];

        if (a == Action::Elevator) {
            if (player_elevators.size() == prm->n_add_elevators) {
                _err << "invalid action: no more elevators available"
                     << std::endl;
                status = status::Error;
                return false;
            } else if (cell == cell_t::Elevator) {
                _err << "invalid action: an elevator is already there"
                     << std::endl;
                status = status::Error;
                return false;
            } else {
                Elevator new_elev { clone.pos, clone.floor };
                player_elevators.push_back(new_elev);
                m_clones.pop_front();
                ++elevators_used;
            }
        } else if (a == Action::Block) {
            blocked_clones.push_back(reverse(clone));
            m_clones.pop_front();
        }

        return true;
    }

    void post_input()
    {
        advance_clones();

        if (!m_clones.empty()
            && m_clones.front().floor == prm->exit_floor
            && m_clones.front().pos == prm->exit_pos) {
            status = status::Won;
        }
    }

private:
    std::vector<cell_t> m_grid;
    std::deque<Clone> m_clones;
    std::vector<Elevator> player_elevators;
    std::vector<Clone> blocked_clones;
    const GameParams* prm;
    int width;
    int height;
    int n_turns;
    int elevators_used;
    int clones_spawned;
    int spawn_cd = 0;

    void advance_clones()
    {
        for (auto& clone : m_clones) {
            if (at_elevator(clone)) {
                ++clone.floor;
            } else {
                if (should_reverse(clone))
                    reverse(clone);

                clone.pos = pos_front(clone);
            }
        }

        if (!m_clones.empty() && at_wall(m_clones.front())) {
            m_clones.pop_front();
        }
    }

    bool at_wall(const Clone& c)
    {
        int mod = c.pos % width;
        return mod == 0 || mod == width;
    }

    bool at_elevator(const Clone& c)
    {
        return (std::find_if(prm->elevators.begin(), prm->elevators.end(), [&c](const auto& e) {
            return e.floor == c.floor && e.pos == c.pos;
        }) != prm->elevators.end()
            || std::find_if(player_elevators.begin(), player_elevators.end(), [&c](const auto& e) {
                   return e.floor == c.floor && e.pos == c.pos;
               })
                != player_elevators.end());
    }

    bool should_reverse(const Clone& c)
    {
        return std::find_if(blocked_clones.begin(), blocked_clones.end(), [&c](const auto& b) {
            return b.floor == c.floor && (b.pos == pos_front(c) || b.pos == c.pos);
        }) != blocked_clones.end();
    }
};

} // namespace dp

using namespace dp;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fmt::print(stderr, "USAGE: {} [Test number]\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string fn;
    fmt::format_to(std::back_inserter(fn), "../data/test{}.txt", argv[1]);

    std::ifstream ifs { fn.data() };

    if (!ifs) {
        fmt::print("Failed to open input file {}", fn);
        return EXIT_FAILURE;
    }

    Game game;
    game.init(ifs);

    DpMgr mgr;
    mgr.load(game);

    assert(mgr.status == DpMgr::status::Initialized);

    while (true)
    {
        mgr.pre_input();

        // output state here

        Action a = Action::Wait;

        mgr.input(a);

        mgr.post_input();

        enum DpMgr::status status = mgr.status;

        switch(status) {
            case DpMgr::status::Won:
                std::cout << "Success!"
                    << std::endl;
                break;
            case DpMgr::status::Lost:
                std::cout << "Game over"
                    << std::endl;
                break;
            case DpMgr::status::Error:
                std::cout << "Error"
                    << std::endl;
                break;
            default:
                continue;
        }
    }

    return 0;
}
