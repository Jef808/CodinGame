#include "agent.h"
#include "breakthrough.h"
#include "eval.h"

#include <cassert>
#include <iostream>
#include <fstream>

    Agent::Agent(Game& _game)
        : game(_game)
    {
        move_buf.reserve(max_n_moves);
    }

    // To be called right after turn_init
    void Agent::make_root() {
        root_moves.clear();
        auto [beg, end] = game.valid_moves();
        std::transform(beg, end, std::back_inserter(root_moves), [](const auto& m){
            return ExtMove(m);
        });
    }

    Move Agent::simple_best_move()
    {
        make_root();

        int best_score = -32001;
        Move best_move = Move_None;

        StateInfo st;
        for (const auto& rm : root_moves) {
            game.apply(rm, st);
            int score = -Eval::evaluate(game);

            std::cerr << "move "
                << Game::view_move(rm) << '\n'
                << game.view()
                << "\nScore: "
                << score << std::endl;

            if (score > best_score) {
                best_score = score;
                best_move = rm;
            }
            game.undo(rm);
        }

        std::cerr << "Best move: "
            << Game::view_move(best_move)
            << ", value: "
            << best_score << std::endl;

        return best_move;
    }

    Move Agent::best_move(int s_depth, int s_width)
    {
        //int best_score = eval_minimax(alpha, beta, s_depth, s_width, ss);
        Move best_move = simple_best_move();

        Stack stack[max_depth];
        Stack* ss = &stack[0];
        ss->depth = 0;
        n_evals = 0;

        for (int depth = 0; depth < s_depth; ++depth) {

            simple_eval_minimax(depth, ss);

            for (const auto& rm : root_moves) {
                std::cerr << Game::view_move(rm.move)
                    << ": "
                    << rm.value
                    << '\n';
            }
            std::cerr << std::endl;

            std::stable_sort(root_moves.begin(), root_moves.end());
        }

        return root_moves[0];
    }

    int Agent::simple_eval_minimax(int s_depth, Stack* ss)
    {
        const bool at_root = ss->depth == 0;
        ++n_evals;

        if (game.is_won())
        {
            if (at_root)
                return 32000;
            return -32000 + ss->depth;
        }

        std::array<Move, max_n_moves> moves;
        moves.fill(Move_None);

        game.compute_valid_moves();
        auto [beg, end] = game.valid_moves();
        std::copy(beg, end, moves.begin());

        // terminal state should get detected at the is_won() check
        assert(std::distance(beg, end) > 0);

        int best_score = -32001;
        Move best_move = Move_None;
        ss->move_count = 0;

        for (const auto& m : moves) {
            if (m == Move_None)
                break;

            ++ss->move_count;

            StateInfo st{};
            (ss + 1)->depth = ss->depth + 1;
            game.apply(m, st);

            int score = -32001;

            if (s_depth == 0) {
                // Only evaluate the move statically at depth 0
                int score = Eval::evaluate(game);
            }
            else {
                // Call minimax recursively at >0 depth
                int score = -simple_eval_minimax(s_depth - 1, ss + 1);
            }

            game.undo(m);

            std::cerr << Game::view_move(m) << ": " << score << std::endl;
            assert( -32001 < score  && score < 32001 );

            // Update the search results if we just found a new best move
            // or if this is the first move checked
            if (score > best_score || ss->move_count == 1) {
                best_score = score;
                best_move = m;

                if (at_root) {
                    ExtMove& rm = *std::find(root_moves.begin(),
                                             root_moves.end(), best_move);
                    rm.value = best_score;
                    std::swap(root_moves[0], rm);
                }
            }
        }

        return best_score;
    }

    // int Agent::eval_minimax(int alpha, int beta, int s_depth, int s_width, Stack* ss)
    // {
    //     const bool at_root = ss->depth == 0;
    //     ss->move_count = 0;
    //     ++n_evals;

    //     /// game.is_won() returns true if the *last player* to have played won, so
    //     /// we should return a positive number when at root and negative otherwise.
    //     /// We include a penalty for being deep in the tree, in case there is a
    //     /// shorter win earlier
    //     if (game.is_won())
    //     {
    //         if (at_root)
    //             return 32000;

    //         return -32000 + ss->depth;
    //     }

    //     std::array<Move, max_n_moves> moves;
    //     moves.fill(Move_None);

    //     game.compute_valid_moves();
    //     auto [beg, end] = game.valid_moves();
    //     std::copy(beg, end, moves.begin());

    //     assert(std::distance(beg, end) > 0);

    //     std::cerr << std::endl;

    //     if (s_depth == 0) {

    //         // TODO Do a static evaluation of the position
    //         // (Like checking for instant obvious things wins
    //         // to avoid generating the moves)
    //         int best_score = -32001;

    //         for (const auto& m : moves) {

    //             if (m == Move_None)
    //                 break;

    //             int score = Eval::score_move(game, m);

    //             if (score > best_score) {
    //                 best_score = score;
    //             }

    //             /// If this is a fresh search, we simpy populate the initial values of the root moves
    //             if (at_root) {
    //                 ExtMove& rm = *std::find(root_moves.begin(), root_moves.end(), m);
    //                 rm.value = score;
    //             }
    //             else {
    //                 // If we found a score so big that the opponent would simply avoid this node altogether,
    //                 // there is no need to further search the other moves
    //                 if (score >= beta)
    //                     return score;
    //             }
    //         }

    //         // We scale down the return value by the current depth. Indeed, we could be very far out in the tree
    //         // so that a very high local score (say a win in two moves) could be much worse than
    //         // a smaller score (say a maybe win in 6) happening at much earlier depth
    //         if (best_score > alpha)
    //             return std::max(alpha, best_score - (ss->depth + 1));  // TODO hardcode some enum values for win in 1,
    //                                                                    // win in 2, etc... along with countering those
    //         return best_score;
    //     }

    //     StateInfo st;
    //     (ss+1)->depth = ss->depth + 1;

    //     int best_score = -32001;
    //     Move best_move = Move_None;

    //     // General depth
    //     for (const auto& m : moves) {

    //         // TODO Actually include the above loop in here, we can probably avoid appying a bunch of moves here
    //         // In particular, we could avoid the whole problem of checking for terminal states on entering the
    //         // search function and some of the mess it creates with depth and signs

    //         if (m == Move_None)
    //             break;

    //         ++ss->move_count;

    //         game.apply(m, st);

    //         int score = -eval_minimax(-beta, -alpha, s_depth - 1, s_width, ss + 1);

    //         game.undo(m);

    //         assert( -32001 < score  && score < 32001 );

    //         // Update the root moves in case we just found a new best move
    //         if (at_root) {

    //             ExtMove& rm = *std::find(root_moves.begin(),
    //                                      root_moves.end(), best_move);

    //             // If this is was first move we explored we save it, otherwise
    //             // only if its best one so far. In the end, we will only have one
    //             // root move with a non-minus-infinity score value
    //             if (ss->move_count == 1 || score > alpha) {

    //                 rm.value = score;
    //             }
    //             else
    //                 rm.value = -32001;

    //             rm.value = best_score;
    //             std::swap(root_moves[0], rm);
    //         }

    //         // Update the search results
    //         if (score > best_score) {

    //             best_score = score;

    //             if (score > alpha) {

    //                 best_move = m;

    //                 if (score < beta)

    //                     alpha = score;  // Tighten up the window
    //                 else
    //                 {
    //                     break;  // beta cutoff: the score was so that the
    //                             // opponent would skip this node with best play
    //                 }
    //             }
    //         }

    //     }
    //     // Went through all moves now
    //     // NOTE: We could make the necessary checks for when there was no moves available here.
    //     // Be there that there's a bug or the game is already won. In Stockfish, they also update
    //     // meta-search statistics here since in case the search failed to prune this node even though
    //     // there is no move available, it is often because of critical events in the game.
    //     // This is why it's so useful to have a `move_count` variable in the search stack.
    //     // It would also be useful to keep track of a principal variation in order to always start
    //     // with the best line so far.
    //     //
    //     // NOTE: A relevant node having only one child (say move_count == 1 here) can be
    //     // contracted into the edge to its parent. In Stockfish it seems to be the
    //     // `continuation history`

    //     return best_score;
    // }
