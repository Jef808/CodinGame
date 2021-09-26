#include "agent.h"
#include "tb.h"
#include "ttable.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstring>
#include <deque>
#include <iostream>
#include <memory>

namespace tb {

namespace {

    struct TimeUtil {
        typedef std::chrono::steady_clock Clock;
        typedef Clock::time_point Point;

        TimeUtil() = default;
        void set_limit(int lim_ms) { limit = lim_ms; }
        void start() { m_start = Clock::now(); }
        bool out() {
            auto ret = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - m_start).count()
                > limit;
            if (ret) std::cerr << "Time out after "
                         << std::chrono::duration<double>(Clock::now() - m_start).count()
                         << " Seconds" << std::endl;
            return ret;
        }

        Point m_start;
        int limit;
    };

    TimeUtil Time;

    bool _timeout = false;
    Value eval(Game& g, Search::Stack* ss, int depth, bool& timeout = _timeout);

} // namespace

// If we used "Speed" or "Slow", we shouldn't use the other one
// before "Jump", "Up" or "Down"
// Action excluded() {
//     Action ret = Action::None;
//     if (actions_hist.empty())
//         return ret;

//     auto q = actions_hist.rbegin();
//     for (; q != actions_hist.rend(); ++q) {
//         if (*q == Action::Speed)
//             return Action::Slow;
//         if (*q == Action::Slow)
//             return Action::Speed;
//     }
//     return Action::None;
// }

Value Score(Game& g) {
    if (g.is_lost())
        return Value::Known_loss;
    if (g.is_won())
        return Value::Known_win;
    return Value(500 * (g.pos() / g.turn()+1 - g.road_length() / 50)
                 + g.ratio_bikes_left());
}

void Agent::play_rootaction(const Action a) {
    auto ra = std::find(ractions.begin(), ractions.end(), a);
    auto&& tmp = std::move(ra->best_line);

    ractions.clear();
    game.apply(a, *st++);
    ++depth_root;

    auto new_best = std::find(ractions.begin(), ractions.end(), tmp[depth_root]);

    std::swap(new_best->best_line, tmp);
}

void Agent::play_rootaction2() {
    assert(ss->depth == depth_root);

    // Apply the action at the base of our current
    // best variation
    game.apply(*ss->best, *st++);

    // Increment the depth
    ++depth_root;

    // Adust the search stack
    ss->depth = depth_root;    // NOTE: We don't actually move the stack pointer,
                               // only change the 'depth' label its pointing at.
    ss->best = (ss+1)->best;
    ss->cur_action = Action::None;
    ss->action_count = 0;



    // Reinitialize the Rootaction objects, and move over
    // the best variation we had over that action `action`.
    // RootActions nex_ractions{};
    // const auto& actions = game.candidates();
    // for (const auto a : actions) {
    //     auto& nex_ra = nex_ractions.emplace_back(action);
    //     if (a == action) {
    //         nex_ra.value = ss->val = ra->value;
    //         nex_ra.depth_completed = ra->depth_completed;
    //         nex_ra.best_line.reserve(ra->best_line.size());
    //         std::copy(ra->best_line.begin() + 1, ra->best_line.end(), std::back_inserter(nex_ra.best_line));
    //     }
    // }

    // std::swap(nex_ractions, ractions);
}

void Agent::init(size_t pow2) {
    // Allocate memory for the transposition table
    //TT.resize(pow2);

    // zero out the search and states stacks.
    std::fill(stack.begin(), stack.end(), 0);
    std::fill(states.begin(), states.end(), 0);

    depth_root = 0;

    // Start the best variation in ss->best by quickly
    // inspecting each one of the root's children
    for (auto a : game.candidates())
    {
        auto& ra = ractions.emplace_back(a);
        game.apply(a, *st);
        ra.value = Score(game);
        game.undo();
        ra.depth_completed = depth_root;
    }

    // Keep the best raction to the front
    std::swap(ractions[0], *std::max_element(ractions.begin(), ractions.end()));
    depth_completed = depth_root;

    // Setup the timer
    Time.set_limit(time_limit);
}

Action Agent::get_next() {
    Time.start();
    bool timeout;

    if (found_winner) {
        Action a = *ss->best;
        play_rootaction2();
        return a;
    }

    assert(!ractions.empty());

    depth_completed = std::min_element(ractions.begin(), ractions.end(),
                                       [](const auto& ra, const auto& rb) {
                                           return ra.depth_completed < rb.depth_completed;
                                       })->depth_completed;

    while (true)
    {
        // Search at the next depth
        int depth = depth_completed + 1;

        Value best_value = ractions[0].value;
        RootAction* best_raction = &ractions[0];

        for (auto& ra : ractions) {
            ss->depth = depth;
            ss->best = &ra.best_line[0];
            ss->depth_best = depth_root + ra.best_line.size();
            ss->cur_action = ra.best_line[0];
            ss->val_best = ra.value;

            // NOTE: As we recurse down from depth to depth_root, the various
            // cur_action's still live on their respective call-stack frames.
            // Thus if we need to update the best_line for ra, we do it inside of
            // eval through ss.
            ra.value = eval(game, ss, depth, timeout);

            // NOTE: Need to think about when to initialize the root actions...
            // IDEA: Could output the actions to std::cout from here until a winning line is found!
            if (timeout) {
                std::cerr << "Timed out during depth "
                    << depth << " search!" << std::endl;

                return ractions[0].best_line[0];
            }

            if (ra.value > best_value) {
                best_value = ra.value;
                best_raction = &ra;
            }

            ++ra.depth_completed;
        }

        std::stable_sort(ractions.begin(), ractions.end());

        ++depth_completed;

        if (Time.out() || ractions[0].value == Value::Known_win)
            return ractions[0].best_line[0];
        // if (depth_completed == 3 || ractions[0].value == Value::Known_win || ractions.size() == 1)
        //     break;
    }
}

namespace {

    /**
     * Do an iterative search so that we can output a decent
     * solution in time even if we don't solve the game
     * before the turn time limit.
     */
    Value eval(Game& g, const Action a, int depth)
    {
        State st;
        g.apply(a, st);

        auto [win, loss] = std::make_pair(g.is_won(), g.is_lost());

        if (win) {
            Value ret = Value::Known_win;
            g.undo();
            return ret;
        }
        else if (loss) {
            Value ret = Value::Known_loss;
            g.undo();
            return ret;
        }

        // depth == 0 means it is beyond the TTEntries
        if (depth == 0) {
            Value ret = Score(g);
            g.undo();
            return ret;
        }

        actions_hist.push_back(a);

        // TODO: Need to allocate space for this outside of the function
        std::array<Action, Max_actions> actions;
        std::fill(actions.begin(), actions.end(), Action::None);

        const auto& cands = g.candidates();
        std::copy(cands.begin(), cands.end(), actions.begin());

        // Exclude the bad choice
        auto it = std::find(actions.begin(), actions.end(), excluded());
        if (it != actions.end())
            *it = Action::None;

        // If that was the only left, we're on a stupid path
        if (cands.size() == 1 && cands[0] == excluded()) {
            Value val = Value(-5000);
            g.undo();
            actions_hist.pop_back();
            return val;
        }

        Value best_value = -Value::Infinite;
        Action best_action = Action::None;

        TTEntry* tte;
        bool tt_found;
        TT.probe(g.key(), tt_found);

        if (tt_found) {
            auto it = std::find(actions.begin(), actions.end(), tte->action());
            if (it != actions.end())
                std::swap(actions[0], *it);
            best_action = actions[0];
            best_value = tte->value();
        }

        for (auto a : actions) {
            if (a == Action::None)
                continue;
            Value val = eval(g, a, depth - 1);
            if (val == Value::Known_loss)
                continue;
            if (val == Value::Known_win) {
                best_value = val;
                best_action = a;
                break;
            }
            if (val > best_value) {
                best_value = val;
                best_action = a;
            }
        }

        // Update the ttEntry in case it was found but has been beaten
        if (tt_found && tte->action() != best_action) {
            tte->save(g.key(), best_value, depth, best_action);
        }

        g.undo();
        actions_hist.pop_back();
        return best_value;
    }
}



} //namespace tb
