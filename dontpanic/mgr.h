#ifndef MGR_H_
#define MGR_H_

#include "dp.h"

#include <deque>
#include <iosfwd>
#include <vector>

namespace dp {

struct Data {
    using iterator = std::vector<Entity>::const_iterator;
    std::vector<Entity> entities;
    int n_clones;
    int n_player_elevators;
    int n_blocked_clones;
};

class DpMgr
{
public:
    enum class status { Uninitialized,
        Initialized,
        Ongoing,
        Lost,
        Won,
        Error } status;

    void load(const Game& game);

    bool pre_input();

    bool input(Action a, std::ostream& _err);

    void post_input();

    const Data* dump() const;

private:

    std::vector<cell_t> m_grid;
    std::deque<Entity> m_clones;
    std::vector<Entity> player_elevators;
    std::vector<Entity> blocked_clones;
    const GameParams* prm;

    mutable Data data;

    int width;
    int height;
    int n_turns;
    int elevators_used;
    int clones_spawned;
    int spawn_cd;

    void advance_clones();
    bool at_wall(const Entity& c);
    bool at_elevator(const Entity& c);
    bool at_blocked(const Entity& c);
    bool should_reverse(const Entity& c);

};


}  // namespace dp


#endif // MGR_H_
