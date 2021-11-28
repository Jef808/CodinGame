#include "eval.h"
#include "breakthrough.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <type_traits>
#include <string>
#include <sstream>


namespace Eval {

namespace {
    enum class Tracing { None, Trace };
}

namespace Bonus {

    /**
     * Bonus point for a lever by rank
     *
     * NOTE: a lever at low-mid rank removes valuable opponent pawn
     * if he captures (can most likely be recaptured too) and lets us
     * disrupt their pawn structure if needed
     */
    constexpr int Lever[height] { 0, 0, 30, 15, 0, 0, 0, 0 };

    /**
     * Bonus points per supporting pawn
     */
    constexpr int Support { 17 };

    /**
     * Bonus points for a having two pawns side by side per rank
     *
     * NOTE: -3 on rank 3 is to avoid biaising the first few moves
     * Should experiment with this, in particular, it should be
     * dynamically changing since as the game goes on, pawns move up
     * and phalanx are important for defense
     */
    constexpr int Phalanx[height] { 7, 8, 12, 21, 25, 100, 250, 0 };

    constexpr int passed_pawned[height] { 0, 0, 0, 0, 0, 32000 - 3, 32000 - 1, -32000 };

} // namespace Bonus


template<Tracing T = Tracing::None>
class Evaluation {
public:

    Evaluation() = delete;
    Evaluation& operator=(const Evaluation&) = delete;
    Evaluation(const Game& _game)
        : game(_game)
    {
    }

    int score();
    std::string view_trace();

private:
    const Game& game;
    std::string debug_buf;

    template<Player P>
    int evaluate_passed_pawns();
    template<Player P>
    constexpr int lever_bonus(const Square& s);
    template<Player P>
    constexpr int connected_bonus(const Square& s);
};

    /**
     * Bonus points for a pawn giving rise to a lever
     *
     * i.e. a pawn having an opponent's pawn diagonally
     */
    template<Tracing T> template<Player P>
    constexpr int Evaluation<T>::lever_bonus(const Square& s) {
        bool lever = false;
        for (auto dx : { -1, 1 })
            lever |= game.cell_at(
                s.col + dx,
                s.row + (P == Player::White ? 1 : - 1)) == cell_of(!P);
        return lever * Bonus::Lever[relative_row(P, s.row)];
    }

    /**
     * Bonus point for connected pawns (supported and/or having neighbours)
     *
     * Parameters are
     * 1. Whether or not it has a neighbour on same rank (phalanx)
     * 2. Number of supporting pawns (0, 1 or 2)
     * 3. (Relative) rank of the pawn
     */
    template<Tracing T> template<Player P>
    constexpr int Evaluation<T>::connected_bonus(const Square& s) {
        int support = 0;
        bool phalanx = false;
        for (const auto dx : { -1, 1 }) {
            support += game.cell_at(
                s.col + dx,
                s.row + (P == Player::White ? -1 : 1)) == cell_of(P);
            phalanx |= game.cell_at(
                s.col + dx,
                s.row) == cell_of(P);
        }
        bool opposed = false;
        for (auto [it, end] = game.pawns_of(!P); it != end; ++it) {
            if (it->col == s.col) {
                opposed = true;
                break;
            }
        }
        return support * Bonus::Support + phalanx * Bonus::Phalanx[relative_row(P, s.row)];
    }

    template<Tracing T> template<Player P>
    int Evaluation<T>::evaluate_passed_pawns() {
        int score = 0;
        // Check for lost game
        for (int x = 0; x < width; ++x) {
            if (game.cell_at(x, relative_row(P, 0)) == cell_of(!P)) {
                score = Bonus::passed_pawned[7];
                break;
            }
        }
        for (auto [it, end] = game.pawns_of(P); it != end; ++it) {
            if (relative_row(P, it->row) == 6 && game.player_to_move() == P) {
                // We win this turn
                score = Bonus::passed_pawned[6];
                break;
            }
        }
        return score;
    }


    template<>
    int Evaluation<Tracing::None>::score() {

        int score = 0;
        {
            int wpp_eval = evaluate_passed_pawns<Player::White>();
            int bpp_eval = evaluate_passed_pawns<Player::Black>();

            if (std::max(abs(wpp_eval), abs(bpp_eval)) >= 32000 - max_depth) {
                int ret = game.player_to_move() == Player::White ? wpp_eval : bpp_eval;
                return ret;
            }

            score += wpp_eval - bpp_eval;
        }

        score += game.board_score() + game.material_imbalance();

        for (auto [it, end] = game.pawns_of(Player::White); it != end; ++it) {
            score += connected_bonus<Player::White>(*it);
            score += lever_bonus<Player::White>(*it);
        }

        for (auto [it, end] = game.pawns_of(Player::Black); it != end; ++it) {
            score -= connected_bonus<Player::Black>(*it);
            score -= lever_bonus<Player::Black>(*it);
        }

        return game.player_to_move() == Player::White ? score : -score;
    }

    template<>
    int Evaluation<Tracing::Trace>::score() {

        std::ostringstream out;

        out << "************\nEvaluation of "  << game.view()
            << "\n    pawn_table score: "      << relative_score(game.player_to_move(), game.board_score())
            << "\n    material_imbalance: "    << relative_score(game.player_to_move(), game.material_imbalance());

        int score = game.board_score() + game.material_imbalance();

        int wpp_eval = evaluate_passed_pawns<Player::White>();
        int bpp_eval = evaluate_passed_pawns<Player::Black>();

        if (std::max(abs(wpp_eval), abs(bpp_eval)) >= 32000 - 1) {
            int ret = game.player_to_move() == Player::White ? wpp_eval : bpp_eval;

            out << "\n\n        WHITE:"
                << "\nevaluate_passed_pawns: " << wpp_eval
                << "\n\n        BLACK:"
                << "\nevaluate_passed_pawns: " << bpp_eval
                << "\n\nRETURNING "            << ret
                << " EARLY\n";

            debug_buf = out.str();
            return ret;
        }

        int wconnected = 0, bconnected = 0,
            wlever     = 0, blever     = 0;

        for (auto [it, end] = game.pawns_of(Player::White); it != end; ++it) {
            wconnected += connected_bonus<Player::White>(*it);
            wlever     += lever_bonus<Player::White>(*it);
        }

        for (auto [it, end] = game.pawns_of(Player::Black); it != end; ++it) {
            bconnected += connected_bonus<Player::Black>(*it);
            blever     += lever_bonus<Player::Black>(*it);
        }

        score += wpp_eval - bpp_eval + wconnected - bconnected + wlever - blever;

        out << "\n\n        WHITE:"
            << "\n    evaluate_passed_pawns: " << wpp_eval
            << "\n    connected_bonus: "       << wconnected
            << "\n    lever_bonus: "           << wlever
            << "\n    WHITE TOTAL: "           << wpp_eval + wconnected + wlever
            << "\n\n"
            << "\n        BLACK:"
            << "\n    evaluate_passed_pawns: " << -bpp_eval
            << "\n    connected_bonus: "       << -bconnected
            << "\n    lever_bonus: "           << -blever
            << "\n    BLACK TOTAL: "           << -(bpp_eval + bconnected + blever)
            << "\n"
            << "\nTOTAL SCORE RETURNED: "      << relative_score(game.player_to_move(), score)
            << '\n';

        debug_buf = out.str();
        return relative_score(game.player_to_move(), score);
    }

    template<Tracing T>
    std::string Evaluation<T>::view_trace() {
        return debug_buf;
    }

} //namespace Eval

int Eval::evaluate(const Game& game) {
    return Evaluation<>(game).score();
}

std::pair<int, std::string> Eval::trace(const Game& game) {
    Evaluation<Tracing::Trace> eval{ game };
    int score = eval.score();
    return { score, eval.view_trace() };
}
