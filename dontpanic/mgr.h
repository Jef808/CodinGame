#ifndef MGR_H_
#define MGR_H_

#include "dp.h"
#include <deque>
#include <iosfwd>
#include <memory>
#include <vector>

namespace dp {

struct DpData
{
    std::vector<Clone> clones;
    std::vector<Elevator> player_elevators;
    std::vector<Clone> blocked_clones;
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

    void load(Game& game);

    bool pre_input();

    bool input(Action a, std::ostream& _err);

    void post_input();

    std::shared_ptr<DpData> dump_data();

private:

    std::vector<cell_t> m_grid;
    std::deque<Clone> m_clones;
    std::vector<Elevator> player_elevators;
    std::vector<Clone> blocked_clones;
    const GameParams* prm;

    std::shared_ptr<DpData> shared_data;

    int width;
    int height;
    int n_turns;
    int elevators_used;
    int clones_spawned;
    int spawn_cd = 0;

    void advance_clones();
    bool at_wall(const Clone& c);
    bool at_elevator(const Clone& c);
    bool should_reverse(const Clone& c);

};

}  // namespace dp


#endif // MGR_H_
