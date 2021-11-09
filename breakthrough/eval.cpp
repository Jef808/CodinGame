#include "eval.h"
#include "breakthrough.h"

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

namespace Eval {

    bool is_obv_passed_pawn(const Game& game, const Square& s) {

        std::cerr << game.view()
            << "is_obv_passed_pawn("
            << Game::view_square(s)
            << "):" << std::endl;

        const Cell c = game.cell_at(s);

        // +1 if cell is white, -1 if cell is black
        const int pm = 1 - 2 * (c == Cell::Black);

        // The possibilities for the first move forward (-1 indicates unavailable move)
        const int forward[3] = {
            // s.col - 1 if not out of bound or blocked by pawn of same color, or -1
            -1 + (s.col) * (game.cell_at(s.col - 1, s.row + pm) != Cell::Boundary)
                             * (game.cell_at(s.col - 1, s.row + pm) != c),
            // s.col if if not blocked, or -1
            -1 + (1 + s.col) * (game.cell_at(s.col, s.row + pm) == Cell::Empty),
            // s.col + 1 if not out of bound or blocked by pawn of same color, or -1
            -1 + (2 + s.col) * (game.cell_at(s.col + 1, s.row + pm) != Cell::Boundary)
                             * (game.cell_at(s.col + 1, s.row + pm) != c),
        };

        std::cerr << "cols considered forward: ";
        for (auto f : forward) { std::cerr << f << ' '; }
        std::cerr << std::endl;

        // Early return false if the square is blocked from moving
        if (forward[0] + forward[1] + forward[2] == -3)
            return false;

        // 7 - row if cell is white, row if cell is black
        const int dist_to_travel = (7 + 7 * pm) / 2 - s.row * pm;

        std::cerr << "distance to travel: "
            << dist_to_travel << std::endl;

        // Early return true if cell is one row away from the end (and not blocked)
        if (dist_to_travel == 1)
            return true;

        // Check for obvious wins starting with each possible forward moves
        for (auto col : { forward[0], forward[1], forward[2] }) {
            if (col == -1)
                continue;

            bool might_get_captured = false;

            // Search the cone based at (col, s.row +- 1) for pawns that might block the push
            for (int d = dist_to_travel - 2, row = s.row + 2 * pm, cone_width = 1;
                 d >= 0;
                 row += pm, --d, ++cone_width)
            {
                for (int x = std::max(0, col - cone_width);
                     x <= std::min(7, col + cone_width);
                     ++x)
                {
                    if (game.cell_at(x, row) != Cell::Empty) {

                        // If we are one row forward, a pawn directly in front is not a threat
                        if (row == s.row + 2 * pm && x == col)
                            continue;

                        // Otherwise, some more calculations would be necessary to decide if
                        // the pawn can be pushed safely to the last row
                        might_get_captured = true;
                        break;
                    }
                }
                // Break the search early for this cone if found a potential block
                if (might_get_captured)
                    break;
            }

            // Early return true if we searched the whole cone and didn't find a threat
            if (!might_get_captured)
                return true;
        }

        // At this point, none of the cones based at forward[i] worked
        return false;
    }

    int evaluate(const Game& game)
    {
        const bool white_to_move = game.player_to_move() == Player::White;
        const bool black_to_move = !white_to_move;

        // +1 if current player is white, -1 if it is black
        const int pm = 1 - 2 * black_to_move;

        int min_row_white = 7;
        int max_row_white = 0;
        int min_row_black = 7;
        int max_row_black = 0;
        int best_obvious_wpp_row = -1;
        int best_obvious_bpp_row = 8;
        int white_support_number = 0;
        int black_support_number = 0;

        // NOTE: game.pawns_of() uses the same buffer for WHITE or BLACK,
        // so a call with a different player as parameter invalidates the iterators

        auto [w_beg, w_end] = game.pawns_of(Player::White);
        int n_white_pawns = std::distance(w_beg, w_end);

        // White pieces:
        for (auto it = w_beg; it != w_end; ++it) {
            // Check for win
            if (it->row == 7) {
                assert(black_to_move);
                return -32000;
            }
            // Record the most advanced row containing a pawn
            if (it->row > max_row_white)
                max_row_white = it->row;

            // Record the most backwards row containing a pawn
            if (it->row < min_row_white)
                min_row_white = it->row;

            // Record row of the most advanced obvious passed pawn if any
            if (it->row > best_obvious_wpp_row && is_obv_passed_pawn(game, *it))
                best_obvious_wpp_row = it->row;

            // Bonus for pawns which are supported (recapture possible if captured)
            for (auto dx : { -1, 1 })
                white_support_number += game.cell_at(it->col + dx, it->row - 1) == Cell::White;
        }

        auto [b_beg, b_end] = game.pawns_of(Player::Black);
        int n_black_pawns = std::distance(b_beg, b_end);

        // Black pieces:
        for (auto it = b_beg; it != b_end; ++it) {
            // Check for win
            if (it->row == 0) {
                assert(white_to_move);
                return -32000;
            }
            // Record the most advanced row containing a pawn
            if (it->row < min_row_black)
                min_row_black = it->row;

            // Record the most backwards row containing a pawn
            if (it->row > max_row_black)
                max_row_black = it->row;

            // Record row of the most advanced obvious passed pawn if any
            if (it->row < best_obvious_bpp_row && is_obv_passed_pawn(game, *it))
                best_obvious_bpp_row = it->row;

            for (auto dx : { -1, 1})
                black_support_number += game.cell_at(it->col + dx, it->row + 1) == Cell::Black;
        }

        int score = 0;

        int white_dist_to_goal = 7 - best_obvious_wpp_row;  //
        int black_dist_to_goal = best_obvious_bpp_row;

        // If at least one side has an obvious passed pawn
        if (white_dist_to_goal < 7 || black_dist_to_goal < 7) {

            // If distances to goal are equal, player_to_move wins the race
            if (white_dist_to_goal == black_dist_to_goal)

                // Remove the number of plies required for the win from "win score"
                return 32000 - 2 * black_dist_to_goal - 1;

            // If white reaches the end first
            if (white_dist_to_goal > black_dist_to_goal)

                // Add a minus sign if it was black's turn
                return (-pm) * (32000 - 2 * white_dist_to_goal - 1);

            // If black reaches the end first
            else
                return pm * (32000 - 2 * black_dist_to_goal - 1);
        }

        // Bonus for material advantage
        score += pm * (n_white_pawns - n_black_pawns) * 105;

        // Bonus for pawns being supported
        score += pm * (white_support_number - black_support_number) * 50;

        return score;
    }

std::string test_passed_pawn()
{
    std::stringstream ss;
    Game game;

    {
        std::ostringstream oss;
        oss << ".....B.."
            << "....BB.."
            << "...B...."
            << "..W....."
            << "........"
            << "..B....."
            << "........"
            << "..W.....";

        game.set_board(oss.str(), Player::White);
    }

    ss << game.view();

    Square sq = { 2, 0 };
    ss << "Square "
       << Game::view_square(sq)
       << (is_obv_passed_pawn(game, sq) ? " is " : " is not ")
       << "an obvious passed pawn\n";

    ss.clear();
    sq = { 2, 2 };
    ss << "Square "
       << Game::view_square(sq)
       << (is_obv_passed_pawn(game, sq) ? " is " : " is not ")
       << "an obvious passed pawn\n";

    ss.clear();
    sq = { 2, 4 };
    ss << "Square "
       << Game::view_square(sq)
       << (is_obv_passed_pawn(game, sq) ? " is " : " is not ")
       << "an obvious passed pawn\n";

    ss.clear();
    sq = { 3, 5 };
    ss << "Square "
       << Game::view_square(sq)
       << (is_obv_passed_pawn(game, sq) ? " is " : " is not ")
       << "an obvious passed pawn\n";

    ss.clear();
    sq = { 4, 6 };
    ss << "Square "
       << Game::view_square(sq)
       << (is_obv_passed_pawn(game, sq) ? " is " : " is not ")
       << "an obvious passed pawn\n";

    return ss.str();
}

std::string test_score()
{
    std::stringstream ss;
    Game game;

    {
        std::ostringstream oss;
        oss << ".....B.."
            << "....BB.."
            << "...B...."
            << "..W....."
            << "........"
            << "..B....."
            << "........"
            << "..W.....";

        game.set_board(oss.str(), Player::White);
    }

    ss << game.view();

    ss << "Evaluation: "
        << evaluate(game)
        << '\n';

    StateInfo st;

    game.apply({ {2, 4}, {1, 5} }, st);

    ss << game.view();

    ss << "Evaluation: "
        << evaluate(game)
        << '\n';

    game.apply({ {2, 2}, {1, 1} }, st);

    ss << game.view();

    ss << "Evaluation: "
        << evaluate(game)
        << '\n';

    return ss.str();
}

std::string test()
{
    std::ostringstream oss;

    oss << "\nPassed pawn test:\n\n"
        << test_passed_pawn() << '\n';

    oss << "\nEvaluation test:\n\n"
        << test_score() << '\n';

    return oss.str();
}

} // namespace Eval
