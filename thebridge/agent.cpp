#include "agent.h"
#include "actionstack.h"
#include "tb.h"
#include "types.h"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace tb {

std::ostream& operator<<(std::ostream& _out, const VAction& va)
{
    return _out << va.action;
}

namespace {

    using ActionList = std::array<VAction, Max_actions>;

    ActionList sactions[Max_depth + 1];
    ActionList* sa { &sactions[0] };
    State sstates[Max_depth + 1];
    State* st { &sstates[1] };

    const Params* game_params;
    Timer time;

    void view_tree()
    {
        while (*sa->begin() != Action::None) {
            for (const auto& va : *sa) {
                if (va != Action::None)
                    std::cout << "{ " << va.action << ' '
                              << to_int(va.cost) << " }, ";
                std::cout << '\n'
                          << std::endl;
            }
        }
    }

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

    void generate_actions(Game& game, int depth)
    {
        for (auto a : game.candidates1())
            std::transform(game.candidates1().begin(), game.candidates1().end(), sactions[depth].begin(), [](auto a) {
                Cost cost = a == Action::None ? Cost::Infinite : Cost::Unknown;
                return VAction{ a, Cost(cost) };
            });
    }

    Cost depth_first_search(Game& game, ActionList* sa,int depth, Cost cost_glb, bool& timeout);

} // namespace

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
    input_turn(*in, *(st - 1), true);
    game.set(*(st - 1));
    game_over = false;

    std::for_each(&sactions[0], &sactions[Max_depth], [](auto& al){
        al.fill(VAction{ Action::None, Cost::Unknown });
    });
}

bool Agent::play_turn()
{
    *out << (*sa++)[0] << std::endl;

    game.apply(*st++, (*sa)[0].action);

    game_over = game.is_lost() || game.is_won();

    if (game_over) {
        std::cerr << "Agent: game is "
                  << (game.is_lost() ? "lost!" : "won!")
                  << std::endl;
        return false;
    }
    if (playing_online) {
        input_turn(*in, *st);
    }

    time.reset();
    return true;
}

/**
 * The greatest lower bound for the number of turns in
 * which it is possible to reach the end of the road.
 */
inline int turn_glb(const Game& g)
{
    int t = 0;
    while (t * (2 * g.get_speed() + t + 1) < 2 * (g.road_length() - g.pos()))
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
 * penalty of 5 turns for each bike lost. Add one for the
 * cost of one turn.
 */
Cost penalty(Game& g, Action a)
{
    static const int& nb_min = game_params->min_bikes;

    int nb_prev = g.n_bikes();
    g.apply(*st, std::move(a));
    g.undo();
    int nb_nex = g.n_bikes();

    return (nb_nex < nb_min || g.turn() > 50)
        ? Cost::Infinite
        : Cost(1 + 5 * (nb_nex - nb_prev));
}

void Agent::solve()
{
    time.reset();
    bool timeout = false;
    Cost best_cost = Cost::Infinite;

    bool last_interrupted = false;
    Cost last_best_cost = Cost::Infinite;
    Action last_best_action = Action::None;
    int last_best_depth = root_depth = 0;

    Cost cost_glb = Cost(turn_glb(game));

    root_depth = 0;
    int search_depth = 0;

    while (++root_depth < Max_depth) {
        while (true) {
            std::cerr << "Searching at depth "
                      << search_depth << std::endl;

            if (sactions[game.turn() + search_depth][0] == Action::None)
                generate_actions(game, game.turn() + search_depth);

            sa = &sactions[game.turn()];

            best_cost = depth_first_search(game, sa, search_depth, cost_glb, timeout);

            /// The stable version of the sort algorithm guarantees to preserve
            /// the ordering of two elements of equal value.
            std::stable_sort(sa->begin(), sa->end());

            if (best_cost <= Cost::Known_win)
                return loop_solved();

            if (timeout) {
                std::cerr << "Agent: Timed out!" << std::endl;
                timeout = false;
                play_turn();
                continue;
            }

            break;
        }

        depth_completed = (search_depth = root_depth);

        if ((*sa)[0] != last_best_action) {
            last_best_action = (*sa)[0].action;
            last_best_cost = (*sa)[0].cost;
            last_best_depth = root_depth;
        }

        /// Check if we have time for the next depth really
        if (time.out()) {
            play_turn();
            time.reset();
        }
    }
}

void Agent::loop_solved()
{
    std::cerr << "Agent: Entering loop_solved()"
              << std::endl;

    sa = &sactions[game.turn()];
    while (true) {
        *out << (*sa++)[0] << std::endl;
        if ((*sa)[0] == Action::None) {
            std::cerr << "Agent: Reached sentinel, exiting program."
                      << std::endl;
            return;
        }
        if (playing_online)
            input_turn(*in, *st++);
    }
}

namespace {

    Cost depth_first_search(Game& game, ActionList* sa, int depth, Cost cost_glb, bool& timeout)
    {
        Cost best_cost = Cost::Infinite;

        if (depth == 0) {
            for (auto& va : *sa) {
                if (va.action == Action::None)
                    continue;

                /// NOTE: Here (at the leaves of the tree) would be a good
                /// place to generate the next list of candidates and
                /// order them!
                va.cost = eval(game) + penalty(game, va.action);

                if (va.cost < best_cost) {
                    best_cost = va.cost;
                    if (va.cost <= Cost::Known_win)
                        break;
                }
                if (va.cost >= Cost::Known_loss) {
                    va.cost = Cost::Infinite;
                }
            }
            /// NOTE: Not a good idea... what if each "pre-leaf" nodes happen to have
            /// the maximum number of children!? We'd be making lots of sporadic bursts
            /// of many calls to time.out() instead of spreading them out.
            if (time.out()) {
                return timeout = true, best_cost;
            }
        } else {
            for (auto& va : *sa) {
                if (va.cost == Cost::Infinite || va.action == Action::None)
                    continue;
                game.apply(*st++, va.action);
                va.cost = depth_first_search(game, sa+1, depth - 1, cost_glb, timeout);
                --st;
                game.undo();
                if (va.cost < best_cost) {
                    best_cost = va.cost;
                    if (va.cost <= Cost::Known_win)
                        break;
                }
                if (timeout) {
                    /// Notice we're querying `timeout` not `time.out()`
                    return timeout = true, best_cost;
                }
                if (va.cost >= Cost::Known_loss) {
                    va.cost = Cost::Infinite;
                }
            }
        }

        /// Make sure the best nodes are always the leftmost ones (in particular,
        /// this is how we will reconstruct the winning sequence if we just found one).
        if ((*sa)[0].cost != best_cost)
            std::swap((*sa)[0], *std::find_if(sa->begin(), sa->end(), [bc=best_cost](const auto& va) {
                return va.cost == bc;
            }));

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
    agent.setup(ifs, std::cout, 150, false);
    agent.solve();

    return EXIT_SUCCESS;
}
