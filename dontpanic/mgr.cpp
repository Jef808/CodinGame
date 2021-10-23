#include "mgr.h"
#include "dp.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>
#include <vector>

namespace dp {

Clone& reverse(Clone& clone)
{
    clone.dir = Clone::Right ? Clone::Left : Clone::Right;
    return clone;
}

int pos_front(const Clone& clone)
{
    return clone.dir == Clone::Right ? clone.pos + 1 : clone.pos - 1;
}

void dp::DpMgr::load(Game& game)
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

bool DpMgr::pre_input()
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

bool DpMgr::input(Action a, std::ostream& _err = std::cerr)
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

void DpMgr::post_input()
{
    advance_clones();

    if (!m_clones.empty()
        && m_clones.front().floor == prm->exit_floor
        && m_clones.front().pos == prm->exit_pos) {
        status = status::Won;
    }
}

void DpMgr::advance_clones()
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

bool DpMgr::at_wall(const Clone& c)
{
    int mod = c.pos % width;
    return mod == 0 || mod == width;
}

bool DpMgr::at_elevator(const Clone& c)
{
    return (std::find_if(prm->elevators.begin(), prm->elevators.end(), [&c](const auto& e) {
        return e.floor == c.floor && e.pos == c.pos;
    }) != prm->elevators.end()
        || std::find_if(player_elevators.begin(), player_elevators.end(), [&c](const auto& e) {
               return e.floor == c.floor && e.pos == c.pos;
           })
            != player_elevators.end());
}

bool DpMgr::should_reverse(const Clone& c)
{
    return std::find_if(blocked_clones.begin(), blocked_clones.end(), [&c](const auto& b) {
        return b.floor == c.floor && (b.pos == pos_front(c) || b.pos == c.pos);
    }) != blocked_clones.end();
}

std::shared_ptr<DpData> DpMgr::dump_data()
{
    shared_data->clones.clear();
    player_elevators.clear();
    blocked_clones.clear();

    std::copy(this->m_clones.begin(), this->m_clones.end(), std::back_inserter(shared_data->clones));
    shared_data->player_elevators = this->player_elevators;
    shared_data->blocked_clones = this->blocked_clones;

    return shared_data;
}

} // namespace dp
