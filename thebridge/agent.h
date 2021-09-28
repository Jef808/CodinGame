#ifndef AGENT_H_
#define AGENT_H_

#include "timeutil.h"
#include "tb.h"


namespace tb {

class Agent {
public:
    static void init();
    Agent() = default;
    void setup(std::istream&, std::ostream&, int timelim_ms = 0, bool online = true);
    void solve();
    int get_root_depth() const { return root_depth; }
private:
    Game game;
    bool use_time;
    bool playing_online;
    int game_depth;
    int root_depth;
    int depth_completed;
    bool game_over;

    std::istream* in;
    std::ostream* out;

    void play_turn();
    void loop_solved();
};



}  // namespace tb

#endif // AGENT_H_
