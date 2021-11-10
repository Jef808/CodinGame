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

    Move Agent::simple_best_move(bool debug_eval)
    {
        make_root();

        int best_score = -32001;
        Move best_move = Move_None;

        StateInfo st;
        for (const auto& rm : root_moves) {

            game.apply(rm, st);

            if (debug_eval) {
                auto [_score, trace] = Eval::trace(game);
                std::cerr << "outputting trace:\n"
                          << trace << std::endl;

                std::cerr << "\nSanity check: value from normal method was "
                    << _score << std::endl;
            }

            int score = -Eval::evaluate(game);

            game.undo(rm);

            if (score > best_score) {
                best_score = score;
                best_move = rm;
            }
        }

        return best_move;
    }

    Move Agent::best_move(int s_depth, int s_width)
    {
        make_root();

        //NOTE: Say I just output a move so that I will win on the next turn.
        // Then main calls best_move again and the root_moves are completely reset.
        // So without any move ordering scheme, the search will just pick a random
        // move first (rn they show up in lexicographic order so bottom-left corner first)
        //
        // But after applying that random action, the eval function will find the pawn that's
        // one row off the end of the board and return "WIN IN ONE"! So then I will classify
        // that action as the best one and only *sometimes* be lucky enough that the actual
        // winning action gets picked.
        //
        // TODO: Keep the best line in memory throughout calls to best_move, or find a way
        // to overwrite that random "winning" action if I find another one that wins faster

        Stack stack[max_depth];
        Stack* ss = &stack[0];
        ss->depth = 0;
        n_evals = 0;

        int alpha = -100;
        int beta = 100;

        Move previous_best_move = root_moves[0];
        int previous_best_score = -32001;

        for (int depth = 0; depth < s_depth; ++depth) {

            int best_score = simple_eval_minimax(s_depth, ss);
            //int best_score = eval_minimax(alpha, beta, s_depth, s_width, ss);

            std::cerr << "depth " << depth
                << " score: " << best_score
                << std::endl;

            std::stable_sort(root_moves.begin(), root_moves.end());

            assert(root_moves[0].value == best_score);

        }

        return root_moves[0];
    }

    int Agent::simple_eval_minimax(int s_depth, Stack* ss)
    {
        const bool at_root = ss->depth == 0;
        ++n_evals;

        bool won_game = game.player_to_move() == Player::White
            ? game.has_won<Player::Black>()
            : game.has_won<Player::White>();

        if (won_game) {
            return at_root ? 32000 : -32000;
        }

        std::array<Move, max_n_moves> moves;
        moves.fill(Move_None);

        game.compute_valid_moves();
        auto [beg, end] = game.valid_moves();
        std::copy(beg, end, moves.begin());

        // terminal state should get detected at the is_won() check
        const int n_moves = std::distance(beg, end);
        assert(n_moves > 0);

        int best_score = -32001;
        Move best_move = Move_None;
        ss->move_count = 0;
        StateInfo st{};
        (ss + 1)->depth = ss->depth + 1;

        for (auto it = moves.begin(); it != moves.begin() + n_moves; ++it) {

            ++ss->move_count;

            game.apply(*it, st);

            int score = best_score;

            if (s_depth == 0) {

                score = -Eval::evaluate(game);

                if (score >= 32000 - max_depth)
                    score -= ss->depth;

                else if (score <= -32000 + max_depth)
                    score += ss->depth;
            }
            else {
                score = -simple_eval_minimax(s_depth - 1, ss + 1);
            }

            game.undo(*it);

            assert( -32001 < score  && score < 32001 );

            // Update the search results if we just found a new best move
            // or if this is the first move checked
            if (score > best_score) {
                best_score = score;
                best_move = *it;

                if (at_root) {
                    ExtMove& rm = *std::find(root_moves.begin(),
                                             root_moves.end(), *it);
                    rm.value = best_score;
                }
            }
        }

        return best_score - ss->depth;
    }

    int Agent::eval_minimax(int alpha, int beta, int s_depth, int s_width, Stack* ss)
    {
        const bool at_root = ss->depth == 0;
        ss->move_count = 0;
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

        const int n_moves = std::distance(beg, end);

        assert(n_moves > 0);

        StateInfo st;
        (ss+1)->depth = ss->depth + 1;

        int best_score = -32001;
        Move best_move = Move_None;

        for (auto it = moves.begin(); it != moves.begin() + n_moves; ++it) {

            ++ss->move_count;

            game.apply(*it, st);
            int score = s_depth == 0
                ? -Eval::evaluate(game)
                : -eval_minimax(-beta, -alpha, s_depth - 1, s_width, ss + 1);
            game.undo(*it);

            assert( -32001 < score  && score < 32001 );

            // Update the root moves in case we just found a new best move
            if (at_root) {
                ExtMove& rm = *std::find(root_moves.begin(),
                                         root_moves.end(), *it);

                if (score > alpha || ss->move_count == 1)
                    rm.value = score;
            }

            // Update the search results
            if (score > best_score) {

                best_score = score;

                if (score > alpha) {

                    best_move = *it;

                    if (score < beta)

                        alpha = score;  // Tighten up the window
                    else
                    {
                        break;  // beta cutoff: the score was so that the
                                // opponent would skip this node with best play
                    }
                }
            }
        }
        // Went through all moves now
        // NOTE: We could make the necessary checks for when there was no moves available here.
        // Be there that there's a bug or the game is already won. In Stockfish, they also update
        // meta-search statistics here since in case the search failed to prune this node even though
        // there is no move available, it is often because of critical events in the game.
        // This is why it's so useful to have a `move_count` variable in the search stack.
        // It would also be useful to keep track of a principal variation in order to always start
        // with the best line so far.
        //
        // NOTE: A relevant node having only one child (say move_count == 1 here) can be
        // contracted into the edge to its parent. In Stockfish it seems to be the
        // `continuation history`

        return best_score;
    }
