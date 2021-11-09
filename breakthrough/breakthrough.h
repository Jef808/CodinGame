#ifndef __BTV2_H_
#define __BTV2_H_

#include "types.h"

#include <array>
#include <iosfwd>
#include <string_view>
#include <vector>


struct StateInfo {
    int ply;
    int move_count;
    Move move;
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
    bool is_won();
    Player player_to_move() const;

    bool is_capture(const Move&) const;
    int turn_number() const;
    static std::string_view view_move(const Move&);
    static std::string_view view_square(const Square&);
    square_range pawns_of(Player) const;
    Cell cell_at(int col, int row) const;
    Cell cell_at(const Square&) const;
    int index_of(const Square&) const;

    /** to be called right after turn_init() */
    bool test_move_gen(bool display = false) const;

private:
    std::array<Cell, (width + 2) * (height + 2)> m_grid;
    int n_turns;
    Player m_player_to_move;
    Player m_player;
    StateInfo* st;

    static Move get_move(std::string_view);
    Cell& cell_at(int col, int row);
    int row_reversed(int row) const;
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
inline Cell& Game::cell_at(int col, int row) {
    return m_grid[(col + 1) + (row + 1) * (width + 2)];
}
inline Cell Game::cell_at(int col, int row) const {
    return m_grid[(col + 1) + (row + 1) * (width + 2)];
}
inline Cell Game::cell_at(const Square& sq) const {
    return m_grid[(sq.col + 1) + (sq.row + 1) * (width + 2)];
}
inline int Game::index_of(const Square& sq) const {
    return sq.col + sq.row * (width);
}
inline int Game::row_reversed(int row) const {
    return height - 1 - row;
}

#endif
