#ifndef AGENT_H_
#define AGENT_H_

#include "tb.h"

namespace tb {

class Agent {
public:
    Agent() = default;
    void setup(std::istream&, std::ostream&, int timelim_ms = 0, bool online = true);
    void solve();

private:
    Game game;
    bool use_time;
    bool playing_online;
    int root_depth;
    int depth_completed;
    bool game_over;

    std::istream* in;
    std::ostream* out;

    bool play_turn();
    void loop_solved();
};

struct VAction {
    VAction()
        : action { Action::None }
        , cost { Cost::Unknown }
    {
    }
    explicit VAction(Action a)
        : action { a }
        , cost { a == Action::None ? Cost::Infinite : Cost::Unknown }
    {
    }
    Action action { Action::None };
    Cost cost { Cost::Unknown };
    inline bool operator<(const VAction& va) const
    {
        return cost < va.cost;
    }
    inline bool operator==(const Action a) const
    {
        return action == a;
    }
    inline bool operator!=(const Action a) const
    {
        return action != a;
    }
    inline bool operator!=(const VAction& va) const
    {
        return action != va.action;
    }
};

extern std::ostream& operator<<(std::ostream& _out, const VAction& va);

} // namespace tb

#endif // AGENT_H_
