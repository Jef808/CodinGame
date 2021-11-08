#include "agent.h"

#include <cassert>
#include <iostream>

    Agent::Agent(Game& _game)
        : game(_game)
    {
        move_buf.reserve(max_n_moves);
    }

    void Agent::make_root() {
        root_moves.clear();
        auto [it, end] = game.valid_moves();
        for (; it != end; ++it) {
            root_moves.push_back(ExtMove{*it});
        }
    }

    Move Agent::simple_best_move(bool debug)
    {
        make_root();
        Move best_move = Move_None;
        int best_val = -32001;
        for (auto& rm : root_moves) {
            rm.value = score_move(rm.move, debug);
            if (debug) {
                std::cerr << game.make_move(rm.move)
                    << ": " << rm.value << '\n';
            }
            if (rm.value > best_val) {
                best_val = rm.value;
                best_move = rm.move;
            }
        }
        if (debug)
            std::cerr << std::endl;

        if (best_move == Move_None) {
            std::cerr << "Agent::simple_best_move: found Move_None..."
                << std::endl;
            assert(false);
        }
        return best_move;
    }

    Move Agent::best_move(int s_width, int s_depth)
    {
        root_moves.clear();
        game.compute_valid_moves();

        auto [beg, end] = game.valid_moves();

        assert(std::distance(beg, end) > 0);

        std::transform(beg, end,
                       std::back_inserter(root_moves),
                       [](auto& m) {
                           return ExtMove(m);
                       });

        Stack stack[max_depth];
        Stack* ss = &stack[0];
        ss->depth = 0;
        n_evals = 0;

        int alpha = -32001;
        int beta = 32001;

        for (int depth = 0; depth <= s_depth; ++depth) {

            int best_score = eval_minimax(alpha, beta, s_depth, s_width, ss);

            std::cerr << "Depth " << depth
                      << ", Score: " << best_score << std::endl;

            // Every root move except one has their value set to -32001.
            // Bring that one to the front without changing the position
            // of the others
            std::stable_sort(root_moves.begin(), root_moves.end());
        }

        std::cerr << "Performed "
                  << n_evals
                  << " calls to the eval method"
                  << std::endl;

        return root_moves[0].move;
    }

    int Agent::eval_minimax(int alpha, int beta, int s_depth, int s_width, Stack* ss)
    {
        const bool at_root = ss->depth == 0;
        ss->move_count = 0;

        /// game.is_won() returns true if the *last player* to have played won, so
        /// we should return a positive number when at root and negative otherwise.
        /// We include a penalty for being deep in the tree, in case there is a
        /// shorter win earlier
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

        assert(std::distance(beg, end) > 0);

        if (s_depth == 0) {

            // TODO Do a static evaluation of the position
            // (Like checking for instant obvious things wins
            // to avoid generating the moves)
            int best_score = -32001;

            for (const auto& m : moves) {

                if (m == Move_None)
                    break;

                int score = score_move(m);

                if (score > best_score) {
                    best_score = score;
                }

                /// If this is a fresh search, we simpy populate the initial values of the root moves
                if (at_root) {
                    ExtMove& rm = *std::find(root_moves.begin(), root_moves.end(), m);
                    rm.value = score;
                }
                else {
                    // If we found a score so big that the opponent would simply avoid this node altogether,
                    // there is no need to further search the other moves
                    if (score >= beta)
                        return score;
                }
            }

            // We scale down the return value by the current depth. Indeed, we could be very far out in the tree
            // so that a very high local score (say a win in two moves) could be much worse than
            // a smaller score (say a maybe win in 6) happening at much earlier depth
            if (best_score > alpha)
                return std::max(alpha, best_score - (ss->depth + 1));  // TODO hardcode some enum values for win in 1,
                                                                       // win in 2, etc... along with countering those
            return best_score;
        }

        StateInfo st;
        (ss+1)->depth = ss->depth + 1;

        int best_score = -32001;
        Move best_move = Move_None;

        // General depth
        for (const auto& m : moves) {

            // TODO Actually include the above loop in here, we can probably avoid appying a bunch of moves here
            // In particular, we could avoid the whole problem of checking for terminal states on entering the
            // search function and some of the mess it creates with depth and signs

            if (m == Move_None)
                break;

            ++ss->move_count;

            game.apply(m, st);

            int score = -eval_minimax(-beta, -alpha, s_depth - 1, s_width, ss + 1);

            game.undo(m);

            assert( -32001 < score  && score < 32001 );

            // Update the root moves in case we just found a new best move
            if (at_root) {

                ExtMove& rm = *std::find(root_moves.begin(),
                                         root_moves.end(), best_move);

                // If this is was first move we explored we save it, otherwise
                // only if its best one so far. In the end, we will only have one
                // root move with a non-minus-infinity score value
                if (ss->move_count == 1 || score > alpha) {

                    rm.value = score;
                }
                else
                    rm.value = -32001;

                rm.value = best_score;
                std::swap(root_moves[0], rm);
            }

            // Update the search results
            if (score > best_score) {

                best_score = score;

                if (score > alpha) {

                    best_move = m;

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

    int Agent::score_move(const Move& move, bool debug) const
    {
        bool white = move.from.row < move.to.row;

        /// If the move is winning instantly
        if (move.to.row == 0 || move.to.row == 7) {
            return 32000;
        }

        /// If the move prevents an instant win next ply
        if ((move.from.row == 0 || move.from.row == 7) && game.is_capture(move)) {
            return 32000 - 1;
        }

        /// The move gives a passed pawn if all cells in
        /// the forward cone based at move.to are empty
        bool passed_pawn = true;
        if (white) {
            for (int row = move.to.row, width = 0; row <= 7; ++row, width += 1) {
                for (int x = std::max(0, move.to.col - width); x <= std::min(7, move.to.col + width); ++x)
                    if (game.cell_at({ x, row }) != Cell::Empty) {
                        passed_pawn = false;
                        break;
                    }
                // break early if found an opponent's piece in the forward cone based at move.to
                if (!passed_pawn)
                    break;
            }
            if (passed_pawn) {
                if (debug) {
                    std::cerr << game.make_move(move)
                              << " gives passed pawn" << std::endl;
                }
                return 32000 - ( 7 - move.to.row );
            }

        }
        else {
            for (int row = move.to.row, width = 0; row >= 0; --row, width += 1) {
                for (int x = std::max(0, move.to.col - width); x <= std::min(7, move.to.col + width); ++x)
                    if (game.cell_at({ x, row }) != Cell::Empty) {
                        passed_pawn = false;
                        break;
                    }
                // break early if found an opponent's piece in the forward cone based at move.to
                if (!passed_pawn)
                    break;
            }
            if (passed_pawn) {
                if (debug) {
                    std::cerr << game.make_move(move)
                              << " gives passed pawn" << std::endl;
                }
                return 32000 - move.to.row;
            }
        }

        /// Non-critical moves follow
        int dist_to_home = white ? move.from.row : 7 - move.from.row;

        // Bias towards captures, and towards backward straight-ahead moves
        int score = (1000 * game.is_capture(move))
            + 100 * (5 - dist_to_home)
            + (10 * (move.from.col == move.to.col));

        return score;
    }
