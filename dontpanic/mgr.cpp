#include "mgr.h"
#include "dp.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <vector>

namespace dp {

Entity& reverse(Entity& clone)
{
    assert(clone.type == Type::Clone);
    clone.dir = std::make_optional(*clone.dir == Dir::Right ? Dir::Left : Dir::Right);
    return clone;
}

int pos_front(const Entity& clone)
{
    assert(clone.type == Type::Clone);
    return *clone.dir == Dir::Right ? clone.pos + 1 : clone.pos - 1;
}

void DpMgr::load(const Game& game)
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
    m_grid[prm->exit_pos + 1 + prm->exit_floor * width] = cell_t::Exit;

    n_turns = elevators_used = clones_spawned = spawn_cd = 0;

    status = status::Initialized;
}

bool DpMgr::pre_input()
{
    ++n_turns;

    if (n_turns > prm->max_round)
        status = status::Lost;

    switch (status) {
    case status::Initialized:
        status = status::Ongoing;
    case status::Ongoing:
        break;
    case status::Won:
    case status::Lost:
    case status::Error:
    case status::Uninitialized:
        return false;
    default:
        assert(false);
    }

    if (clones_spawned < prm->max_clones) {
        if (spawn_cd == 0) {
            Entity spawn { Type::Clone, prm->entry_pos, 0, std::make_optional(Dir::Right) };
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
    assert(a != Action::None);
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

    Entity clone = m_clones.front();
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
            Entity new_elev { Type::Elevator, clone.pos, clone.floor };
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

    // m_grid[m_clones.front().pos + 1 + m_clones.front().floor * width] == cell_t::Exit;
}

void DpMgr::advance_clones()
{
    for (auto& clone : m_clones) {
        if (at_elevator(clone) && !at_blocked(clone)) {
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

bool DpMgr::at_wall(const Entity& c)
{
    return c.pos == -1 || c.pos == width - 2;
}

bool DpMgr::at_elevator(const Entity& c)
{
    return (std::find_if(prm->elevators.begin(), prm->elevators.end(), [&c](const auto& e) {
        return e.floor == c.floor && e.pos == c.pos;
    }) != prm->elevators.end()
        || std::find_if(player_elevators.begin(), player_elevators.end(), [&c](const auto& e) {
               return e.floor == c.floor && e.pos == c.pos;
           })
            != player_elevators.end());
}

bool DpMgr::at_blocked(const Entity& c)
{
    return std::find_if(blocked_clones.begin(), blocked_clones.end(), [&c](const auto& b) {
        return b.floor == c.floor && b.pos == c.pos;
    }) != blocked_clones.end();
}

bool DpMgr::should_reverse(const Entity& c)
{
    return std::find_if(blocked_clones.begin(), blocked_clones.end(), [&c](const auto& b) {
        return b.floor == c.floor && (b.pos == pos_front(c) || b.pos == c.pos);
    }) != blocked_clones.end();
}

const Data* DpMgr::dump() const
{
    data.entities.clear();

    std::copy(m_clones.begin(), m_clones.end(), std::back_inserter(data.entities));
    std::copy(player_elevators.begin(), player_elevators.end(), std::back_inserter(data.entities));
    std::copy(blocked_clones.begin(), blocked_clones.end(), std::back_inserter(data.entities));

    data.n_clones = m_clones.size();
    data.n_player_elevators = player_elevators.size();
    data.n_blocked_clones = blocked_clones.size();

    return &data;
}

} // namespace dp
