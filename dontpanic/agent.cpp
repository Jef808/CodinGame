#include "agent.h"
#include "dp.h"
#include "types.h"

#include <array>
#include <string>

namespace dp {

    /// Globals
    namespace {
        constexpr int max_actions = max_width;

        enum class Cost {
            Zero=0,
            KnownWin=1,
            Unknown=50,
            KnownLoss=99,
            Infinite=200,
        };

        struct Action {
            Action() = default;
        };

        State states[max_turns + 1];
        State* ps = &states[0];
        const GameParams* params;

        using ActionList = std::array<Action, max_actions>;
        std::array<ActionList, max_turns + 1> actions;
        ActionList* as = &actions[0];

        /// The actions to consider at each floor
        std::vector<std::vector<int>> candidates;

        /// Used to populate those actions
        void init_actions();

    }  // namespace


    void Agent::init(const Game& g)
    {
        std::fill(&states[0], &states[max_turns], State{});

        for (int i=0; i<max_turns+1; ++i) {
            std::fill(&actions[i][0], &actions[i][max_actions-1], Action{});
        }

        params = g.get_params();
        *ps++ = *g.state();
        as = &actions[1];

        init_actions();
    }

    namespace {

        bool operator<(const Cost a, const Cost b) {
            return to_int(a) < to_int(b);
        }
        Cost operator+(Cost a, Cost b) {
            return Cost(to_int(a) + to_int(b));
        }

        /// Lower bound on cost (manhattan distance to exit)
        inline Cost cost_lb(const State& s) {
            return Cost(params->exit_pos - s.pos + params->exit_floor - s.floor);
        }

        /// Cost to date
        inline Cost prev_cost(const State& s) {
            return Cost(s.turn);
        }

        /// True if the game is lost
        inline bool is_lost(const State& s) {
            return cost_lb(s) + prev_cost(s) > Cost(params->max_round)
                   || s.used_clones > params->max_clones;
        }

        /// True if the game is won
        inline bool is_won(const State& s) {
            return s.floor == params->exit_floor
                && s.pos == params->exit_pos
                && !is_lost(s);
        }

        /// Only elevators under or one-off of an elevator
        /// above need to be considered
        void init_actions() {
            std::array<std::array<bool, max_width>, max_height> seen;
            for (int i=0; i<max_height; ++i) {
                seen[i].fill(false);
            }
            const int max_gap = params->n_add_elevators;

            /// Start at the top
            for (auto el = params->elevators.rbegin(); el != params->elevators.rend(); ++el)
            {
                /// Considers all three columns adjacent to the elevator
                for (int i=el->pos - 1; i<el->pos+2; ++i)
                {
                    if (i < 0 || i > params->width-1)
                        continue;

                    /// Go downwards
                    for (int h = el->floor; h > (el->floor - max_gap) && h >= 0; --h)
                    {
                        /// Register the positions in reach of the max_gap
                        seen[h][i] = true;
                    }
                }
            }
            /// Save the result in the global vector
            for (int i=0; i<params->height; ++i)
            {
                std::copy(seen[i].begin(), seen[i].begin() + params->width, std::back_inserter(candidates.emplace_back()));
            }
        }

    }  // namespace

    std::string Agent::best_choice()
    {
        return "WAIT";
    }



} // namespace dp
