#ifndef __BTV2_H_
#define __BTV2_H_

#include "types.h"

#include <array>
#include <iosfwd>
#include <string_view>
#include <vector>


struct StateInfo {
    int ply;
    int board_score;
    int mat_imba;
    bool is_capture;
    StateInfo* prev;
};

class Game {
public:
    using valid_move_iterator = std::vector<Move>::const_iterator;
    using valid_move_range = std::pair<valid_move_iterator, valid_move_iterator>;
    using square_iterator = std::vector<Square>::const_iterator;
    using square_range = std::pair<square_iterator, square_iterator>;

    Game();
    void init(std::istream&);
    void turn_init(std::istream&, StateInfo&);
    void set_board(std::string_view, Player to_move);
    std::string_view view() const;
    void apply(const Move&, StateInfo&);
    void undo(const Move&);
    void compute_valid_moves() const;

    valid_move_range valid_moves() const;
    template<Player P>
    constexpr bool has_won() const;
    bool is_won() const;
    Player player_to_move() const;
    constexpr int board_score() const;
    constexpr int material_imbalance() const;

    bool is_capture(const Move&) const;
    int turn_number() const;
    static std::string_view view_move(const Move&);
    static std::string_view view_square(const Square&);
    square_range pawns_of(Player) const;
    constexpr Cell cell_at(int col, int row) const;
    constexpr Cell cell_at(const Square&) const;
    constexpr int index_of(const Square&) const;
    constexpr int row_reversed(int row) const;

    /** to be called right after turn_init() */
    bool test_move_gen(bool display = false) const;

private:
    std::array<Cell, (width + 2) * (height + 2)> m_grid;
    int n_turns;
    Player m_player_to_move;
    Player m_player;
    StateInfo* st;

    static Move get_move(std::string_view);
    constexpr Cell& cell_at(int col, int row);
};

inline bool Game::is_capture(const Move& move) const {
    return cell_at(move.to) == cell_of(!player_to_move());
}
inline int Game::turn_number() const {
    return n_turns;
}
inline Player Game::player_to_move() const {
    return m_player_to_move;
}
constexpr Cell& Game::cell_at(int col, int row) {
    return m_grid[(col + 1) + (row + 1) * (width + 2)];
}
constexpr Cell Game::cell_at(int col, int row) const {
    return m_grid[(col + 1) + (row + 1) * (width + 2)];
}
constexpr Cell Game::cell_at(const Square& sq) const {
    return m_grid[(sq.col + 1) + (sq.row + 1) * (width + 2)];
}
constexpr int Game::index_of(const Square& sq) const {
    return sq.col + sq.row * (width);
}
constexpr int Game::row_reversed(int row) const {
    return height - 1 - row;
}
constexpr int Game::board_score() const {
    return st->board_score;
}
constexpr int Game::material_imbalance() const {
    return st->mat_imba;
}
template<Player P>
constexpr bool Game::has_won() const {
    for (int x = 0; x < width; ++x) {
        if (cell_at(x, relative_row(P, 7)) == cell_of(P))
            return true;
    }
    return false;
}
#endif
