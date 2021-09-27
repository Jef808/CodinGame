#include "tb.h"
#include "timeutil.h"
#include "types.h"

#include <algorithm>
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>

using namespace tb;

const int max_depth = 50;
const int max_n_children = 5;

struct Stack {
    Action* best{ nullptr };
    int depth{ 0 };
    Action cur_action{ Action::None };
    Cost cur_cost{ Cost::Infinite };
};

class Agent {
public:
    using ExtAction = std::pair<Action, Cost>;

    Agent() = default;
    void solve(std::istream&, std::ostream&, bool update_from_input = false);
    void set_timelimit_ms(int t) { time_limit = t; }

private:
    Game game;
    Timer time;
    std::array<State, max_depth + 1> states;
    State* st;
    std::array<Stack, max_depth + 1> stack;
    Stack* ss;
    std::array<Action, max_depth + 1> best_line;
    std::vector<ExtAction> root_actions;

    int root_depth { 0 };
    bool game_over { false };
    int time_limit { std::numeric_limits<int>::max() };

    void init_search();
    void turn_input(std::istream&, bool update_state = false);
    void turn_output(std::ostream& _out, Action);
    void play_best_action(std::ostream&, std::istream&, bool update_from_input = false);
    void set_root_actions(int dfs = -1);
};

/**
 * Sort the ExtActions in increasing order of cost.
 */
bool operator<(const Agent::ExtAction& a, const Agent::ExtAction& b) {
    return a.second < b.second;
}

/**
 * Initialize the data to be used by the agent in its search.
 *
 * NOTE: This should be called before setting up the initial state
 * in case the game allows for more time before the first turn.
 */
void Agent::init_search() {
    /// Zero out the search stacks with default values
    std::fill(stack.begin(), stack.end(), Stack{});
    std::fill(best_line.begin(), best_line.end(), Action::None);

    root_depth = 0;
    root_actions.reserve(max_n_children);

    st = &states[root_depth+1];
    ss = &stack[root_depth+1];
    ss->best = &best_line[root_depth+1];

    time.set_limit(this->time_limit);

    game_over = false;
}

/**
 * Read the turn input from the istream, updating the agent's
 * current game state if the `update_state` flag is set.
 */
void Agent::turn_input(std::istream& _in, bool update_state) {
    if (update_state)
    {
        --st;

        int x, y, a;  // NOTE: x-coord, y-coord, active-or-not
        _in >> st->speed; _in.ignore();

        for (int i=0; i<params.start_bikes; ++i)
        {
            _in >> x >> y >> a; _in.ignore();
            st->bikes[i] = (a == 1);
        }
        st->pos = x;

        ++st;
    }
    else
    {
        std::string buf;
        for (int i=0; i<game.n_bikes_start()+1; ++i)
            std::getline(_in, buf);
    }
}

/**
 * Output the chosen action to the ostream.
 */
void Agent::turn_output(std::ostream& _out, Action a) {
    _out << a << std::endl;
}

/**
 * The real cost spent to date from the initial state:
 * The number of turns elapsed so far.
 *
 * NOTE: The g in A*'s (f = g + h) cost function
 */
inline Cost cost_to_date(Game& g) {
    return Cost(g.turn());
}

/**
 * A lower bound for the remaining cost to spend before
 * reaching a win: Number of turns before end of road
 * if only `Speed` was used as actions.
 *
 * NOTE: The h in A*'s (f = g + h) cost function
 */
Cost cost_remaining_lb(Game& g) {
    int dist = g.road_length() - g.pos();
    int s = g.speed(), n = 0;
    while (dist > 0)
    {
        dist -= ++s;
        ++n;
    }
    return Cost(n);
}

/**
 * Use basic brute-force dfs search to find a tight
 * lower bound for the current game's total cost.
 */
Cost depth_first_search(Game& game, Stack* ss, int depth) {
    assert(depth > 0);

    State st;
    game.apply(ss->cur_action, st);
    ss->cur_cost = (ss-1)->cur_cost + 1;  // NOTE: Assuming every action's base cost is 1

    Cost cost_lb = Cost::Infinite;

    if (depth == 0)
        cost_lb = ss->cur_cost + cost_remaining_lb(game);
    else
    {
        std::array<Action, 5> children = game.candidates1();
        for (auto c : children)
        {
            (ss+1)->cur_action = c;
            Cost _cost = depth_first_search(game, ss+1, depth-1);
            cost_lb =
                _cost < cost_lb
                ? _cost
                : cost_lb;
        }
    }
    game.undo();
    return cost_lb;
}

/**
 * Set the candidate root actions for the next turn,
 * optionally using basic dfs to order set and order their values.
 *
 * NOTE: We use the heavier `game.candidates()` method,
 * which filters the results first, to populate the root actions.
 */
void Agent::set_root_actions(int search_depth) {
    root_actions.clear();

    const auto& cands = game.candidates();
    std::transform(cands.begin(), cands.end(), std::back_inserter(root_actions),
                   [](const auto c){ return std::make_pair(c, Cost::Infinite); });

    for (auto& a : root_actions)
    {
        ss->cur_action = a.first;
        ss->cur_cost = cost_to_date(game);
        a.second = depth_first_search(game, ss, search_depth);
    }

    std::stable_sort(root_actions.begin(), root_actions.end());
}

/**
 * Pick the next action in the agent's current `best_line`,
 * output it to the ostream and apply it to the current game.
 *
 * NOTE: Set the `update_from_input` flag on to also use the input
 * for updating the state (e.g. in multiplayer games).
 * The agent's time counter is also reset.
 */
void Agent::play_best_action(std::ostream& _out, std::istream& _in, bool update_from_input) {
    const Action& best_action = best_line[root_depth+1];
    State& nex_state = states[root_depth+1];

    game.apply(best_action, nex_state);
    ++root_depth;
    ++ss->best;
    ++st;

    turn_output(_out, best_action);
    turn_input(_in, update_from_input);

    time.reset();

    game_over = (game.is_won() || game.is_lost());
}

/** IDA* search */
Value ida_search(Game& game, Stack* ss, int lbound)
{
    Value value = -Value::Infinite;

    return value;
}

/**
 * Solve the problem, reading each turn's input from the
 * given istream `_in` and sending each turn's chosen action
 * to the given ostream `_out`.
 *
 * NOTE: This function doesn't return until the actual game
 * is over.
*/
void Agent::solve(std::istream& _in, std::ostream& _out, bool update_from_input) {
    init_search();
    turn_input(_in, true);
    time.reset();

    /// The depth at which to search to give prior values
    /// to new root actions
    size_t root_init_search_depth = 2;

    /// We time the initialization of the root actions
    /// to decide how quick we will be to interrupt the
    /// search to output the current best actions later on
    Timer search_timer;
    search_timer.reset();
    set_root_actions(root_init_search_depth);

    Timer::Duration root_init_time = search_timer.elapsed_time();
    Timer::Duration adjusted_time_limit = time_limit - root_init_time;

    set_timelimit_ms(adjusted_time_limit);

    std::cerr << "Initialization time: "
        << root_init_time << " ms,"
        << "\nThe search time is set to "
        << adjusted_time_limit << " per turn." << std::endl;

    bool timeout = time.out();

    /// Main loop: Perform depth first search with an iteratively broadening
    /// criterion for selecting nodes to search. (IDA* algorithm)
    while (!game_over)
    {

    }
}

int main(int argc, char *argv[])
{
    const int max_time = 150;

    if (argc < 2) {
        std::cerr << "Input file needed!" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream ifs { argv[1] };
    if (!ifs) {
        std::cerr << "Failed to open input file!" << std::endl;
        return EXIT_FAILURE;
    }

    Game::init(ifs);

    Agent agent;
    agent.set_timelimit_ms(max_time);
    agent.solve(ifs, std::cout);

    return EXIT_SUCCESS;
}
