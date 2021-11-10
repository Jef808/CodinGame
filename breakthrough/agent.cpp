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

        // NOTE: The problem I always keep recreating is because of what follows:
        // If a move results in a "Win in 1 move" state, I return MAX_SCORE - 1.
        // But after applying that move, because of how the evaluation function works,
        // applying ANY other action that doesn't win instantly also returns MAX_SCORE - 1
        // ( the evaluation just checks for a pawn at the second-to-last row... )
        // So if the root_moves are cleared between the calls to `best_move` and nothing is
        // kept in memory, the agent will simply output actions in the random order they are
        // presented to it. As a result, the opponent can find the time to win before I randomly
        // stumble on the right action for winning myself.
        //
        // SOLUTION: I made sure to early return MAX_SCORE in the minimax() function so that it
        // will overwrite all the other MAX_SCORE - 1 results. However, I still need to search for that
        // right move even though I just found it. What is needed is some kind of history mechanism
        // or a hash table.
        //
        // BASIC IDEA: When overwriting a value in the root_nodes, also save the best follow-up move
        // if it is available as member data of the Agent class.

        Stack stack[max_depth];
        Stack* ss = &stack[0];
        ss->depth = 0;
        n_evals = 0;

        int alpha = -100;
        int beta = 100;

        Move previous_best_move = root_moves[0];
        int previous_best_score = -32001;

        for (int depth = 0; depth < s_depth; ++depth) {

            //int best_score = simple_eval_minimax(s_depth, ss);
            int best_score = eval_minimax(alpha, beta, s_depth, s_width, ss);

            std::cerr << "depth " << depth
                << " score: " << best_score
                << std::endl;

            std::stable_sort(root_moves.begin(), root_moves.end());

            assert(root_moves[0].value == best_score);

        }
        return root_moves[0];
    }


    /**
     * Brute-force minimax implementation.
     *
     * NOTE: This cannot handle depth more than 1 or 2 as it is...
     */
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

    /**
     * The main alpha beta minimax method
     */
    int Agent::eval_minimax(int alpha, int beta, int s_depth, int s_width, Stack* ss)
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
            int score = best_score;

            game.apply(*it, st);
            if (s_depth == 0) {
                score = -Eval::evaluate(game);

                // Adjust scores with a penalty for winning moves that are deep into the tree
                if (score >= 32000 - max_depth)
                    score -= ss->depth;

                else if (score <= -32000 + max_depth)
                    score += ss->depth;
            }
            // General depth
            else
                score = -eval_minimax(-beta, -alpha, s_depth - 1, s_width, ss + 1);

            game.undo(*it);

            assert( -32001 < score  && score < 32001 );

            // Finished searching a branch, update the search results
            if (score > best_score) {

                best_score = score;

                // Found a new best move
                if (score > alpha) {

                    best_move = *it;
                    if (at_root) {
                        // Update the value in case of a root_move
                        ExtMove& rm = *std::find(root_moves.begin(),
                                                 root_moves.end(), *it);
                            rm.value = score;
                    }

                    // If we're deeper in the tree, adjust the data to be propagated while searching
                    // other subbranches, or prune all other subbranches in case of a beta cut-off.
                    if (score < beta)
                        alpha = score;  // Tighten the window
                    else
                        break;
                }
            }
        }
        // Went through all moves now
        // TODO: Save the best move to some kind of pv history
        //
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
