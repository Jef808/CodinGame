#include "agent.h"
#include "tb.h"
#include "types.h"

#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>

namespace tb {

struct VAction {
    Action action;
    Cost cost;
    bool operator<(const VAction& va)
    {
        return cost < va.cost;
    }
    bool operator==(const Action a)
    {
        return action == a;
    }
    bool operator!=(const Action a)
    {
        return action != a;
    }
    bool operator!=(const VAction& va)
    {
        return action != va.action;
    }
};

bool operator<(const VAction& va, const VAction& vb) {
    return vb.cost < va.cost;
}
bool operator==(const VAction& va, const VAction& vb) {
    return va.action == vb.action;
}

std::ostream& operator<<(std::ostream& _out, const VAction& va)
{
    return _out << va.action;
}

namespace {

    std::array<Action, Max_depth + 1> sactions;
    Action* pv { &sactions[1] };

    std::array<State, Max_depth + 1> sstates;
    State* st { &sstates[1] };

    std::deque<std::array<VAction, 5>> scands;
    std::array<VAction, 5>* pcands { &scands[0] };

    const Params* game_params;
    Timer time;

    /**
     * Read the turn input from the istream, updating the agent's
     * current game state if the `update_state` flag is set.
     */
    void input_turn(std::istream& _in, State& st, bool update_state = false)
    {
        if (update_state) {
            int x, y, a; // NOTE: x-coord, y-coord, active-or-not
            _in >> st.speed;
            _in.ignore();

            for (int i = 0; i < game_params->start_bikes; ++i) {
                _in >> x >> y >> a;
                _in.ignore();
                st.bikes[i] = (a == 1);
            }
            st.pos = x;
        } else {
            std::string buf;
            for (int i = 0; i < game_params->start_bikes + 1; ++i)
                std::getline(_in, buf);
        }
    }

    Cost depth_first_search(Game& game, int depth, Cost cost_glb, bool& timeout);

} // namespace

void Agent::init()
{
    sstates.fill(State {});
    sactions.fill(Action::None);
}

void populate_next_cands(Game& game)
{
    int _root_depth = game.handling_agent()->get_root_depth();

    scands.push_back(std::array<VAction, 5> {});

    const auto& game_cands = game.candidates1();

    std::transform(game_cands.begin(), game_cands.end(), scands.back().begin(), [](auto a) {
        Cost cost = a == Action::None ? Cost::Infinite : Cost::Unknown;
        return VAction {
            a,
            cost,
        };
    });
}

void Agent::setup(std::istream& _in, std::ostream& _out, int timelim_ms, bool online)
{
    init();

    in = &_in;
    out = &_out;

    playing_online = online;

    use_time = timelim_ms > 0;
    if (use_time)
        time.set_limit(timelim_ms);

    game_params = game.parameters();

    st = &sstates[1];
    input_turn(*in, *(st-1), true);
    game.set(*(st-1), this);

    game_depth = 0;
    game_over = false;
}

void Agent::play_turn()
{
    Action a = Action::Jump;
    if (scands.empty()) {
        std::cerr << "play_turn() called with no candidates!"
                  << "\n Outputting JUMP by default" << std::endl;
    } else {
        a = scands.front()[0].action;
        scands.pop_front();
    }

    *out << a << std::endl;

    game.apply(a, *st++);
    game_over = game.is_lost() || game.is_won();

    if (game_over) {
        std::cerr << "Agent: game is "
                  << (game.is_lost() ? "lost!" : "won!")
                  << std::endl;
    } else {
        if (playing_online)
            input_turn(*in, *st);
        time.reset();

        ++game_depth;
        --root_depth;
    }
}

/**
 * The greatest lower bound for the number of turns in
 * which it is possible to reach the end of the road.
 */
inline int turn_glb(const Game& g)
{
    int t = 0;
    while (t * (2*g.get_speed() + t + 1) < 2 * (g.road_length() - g.pos()))
        ++t;
    return t;
}

/**
 * Add up the current number of turns elapsed to the least upper bound
 * for the remaining part of the game.
 */
inline Cost eval(const Game& g)
{
    return Cost(g.turn() + turn_glb(g));
}

/**
 * Penalty of Cost::Max for loosing state, and
 * penalty of 5 turns for each bike lost.
 */
Cost penalty(Game& g, Action a)
{
    static const int& nb_min = game_params->min_bikes;

    int nb_prev = g.n_bikes();
    State s;
    g.apply(a, s);
    g.undo();
    int nb_nex = g.n_bikes();

    return (nb_nex < nb_min || g.turn() > 50)
        ? Cost::Infinite
        : Cost( 5 * (nb_nex - nb_prev) );
}

void Agent::solve()
{
    time.reset();

    bool timeout = false;
    Cost best_cost = Cost::Infinite;
    Cost last_best_cost = Cost::Infinite;
    Action last_best_action = Action::None;
    int last_best_depth = root_depth;
    Cost cost_glb = Cost(turn_glb(game));
    Cost delta = Cost(2);

    depth_completed = root_depth = 0;

    while (root_depth < Max_depth) {

        //std::array<VAction, Max_actions> prev_scores;
        //std::copy(pcands->begin(), pcands->end(), prev_scores.begin());

        populate_next_cands(game);
        assert(pcands == &scands[0]);

        while (true) {
            Cost best_cost = depth_first_search(game, root_depth, cost_glb, timeout);

            /// Use a stable version of the algorithm so that actions with equal costs don't
            /// get shuffled
            std::stable_sort(pcands->begin(), pcands->end());

            if (best_cost <= Cost::Known_win)
                loop_solved();

            if (timeout) {
                std::cerr << "Agent: Timed out!"
                          << std::endl;
                play_turn();
                continue;
            }

            // TODO: Use the least upper bound Cost function above to devise
            // a good criterion to break from that loop.
            break;
        }

        depth_completed = ++root_depth;

        if (scands[0][0] != last_best_action) {
            last_best_action = scands[0][0].action;
            last_best_cost = scands[0][0].cost;
            last_best_depth = root_depth;
        }
    }
}

void Agent::loop_solved()
{

    std::cerr << "Agent: Entering loop_solved()"
              << std::endl;

    while (true) {
        assert(pcands);

        *out << (*pcands++)[0] << std::endl;

        if ((*pcands)[0] == Action::None) {
            std::cerr << "Agent: Reached sentinel, exiting program."
                      << std::endl;
            return;
        }

        if (playing_online)
            input_turn(*in, *st);
    }
}

namespace {

    Cost depth_first_search(Game& game, int depth, Cost cost_glb, bool& timeout)
    {
        Cost best_cost = Cost::Infinite;

        for (auto& va : *pcands++) {
            if (va.cost >= Cost::Known_loss) {
                va.cost = Cost::Infinite;
                continue;
            }
            game.apply(va.action, *st++);

            if (depth == 0)
                va.cost = eval(game) + penalty(game, va.action);
            else
                va.cost = depth_first_search(game, depth - 1, cost_glb, timeout);

            game.undo();
            --st;

            if (va.cost < best_cost) {
                best_cost = va.cost;
                if (va.cost == Cost::Known_win)
                    break;
            }
            if (time.out()) {
                --pcands;
                return timeout = true, best_cost;
            }
        }
        if ((*pcands)[0].cost != best_cost)
            std::swap((*pcands)[0], *std::find_if(pcands->begin(), pcands->end(), [&best_cost](auto va){
                        return va.cost == best_cost;
                    }));
        --pcands;
        return best_cost;
    }

} // namespace

} // namespace tb

using namespace tb;

int main(int argc, char* argv[])
{
    const int max_time = 150;

    if (argc < 2) {
        std::cerr << "Main: Input file needed!" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream ifs { argv[1] };
    if (!ifs) {
        std::cerr << "Main: Failed to open input file!" << std::endl;
        return EXIT_FAILURE;
    }

    Game::init(ifs);
    Agent::init();

    Agent agent;
    agent.setup(ifs, std::cout, 0, false);
    agent.solve();

    return EXIT_SUCCESS;
}
