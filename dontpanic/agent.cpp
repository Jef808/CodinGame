#include "agent.h"
#include "dp.h"
#include "types.h"

#include <array>
#include <string>

namespace dp {

    /// Globals
    namespace {

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
        constexpr int max_actions = max_width;

        bool operator<(const Cost a, const Cost b) {
            return to_int(a) < to_int(b);
        }

        State states[max_turns + 1];
        State* ps = &states[0];
        const GameParams* gparams;

        using ActionList = std::array<Action, max_actions>;
        std::array<ActionList, max_turns + 1> actions;
        ActionList* as = &actions[0];

        /// The recursive method
        Cost depth_first_search(Game& g, int depth);

    }  // namespace


    void Agent::init(const Game& g)
    {
        std::fill(&states[0], &states[max_turns], State{});

        for (int i=0; i<max_turns+1; ++i) {
            std::fill(&actions[i][0], &actions[i][max_actions-1], Action{});
        }

        gparams = g.get_params();
        *ps++ = *g.state();
        as = &actions[1];
    }

    namespace {

        /// A cost that implies a loss
        constexpr Cost cost_ub = params->max_round;

        /// Lower bound on cost (manhattan distance to exit)
        inline Cost cost_lb(const State& s) {
            return Cost(params->exit_pos - s.pos + params->exit_floor - s.floor);
        }

        /// Cost to date
        inline Cost prev_cost(const State& s) {
            return s.turn;
        }

        /// True if the game is lost
        inline bool is_lost(const State& s) {
            return cost_lb(s) + prev_cost(s) > cost_ub
                   || s.used_clones > params->max_clones;
        }

        /// True if the game is won
        inline bool is_won(const State& s) {
            return s.floor == params->exit_floor
                && s.pos == params->exit_pos
                && !is_lost(s);
        }

    }

    std::string Agent::best_choice()
    {
        return "WAIT";
    }



} // namespace dp
