#ifndef __TYPES_H_
#define __TYPES_H_

#include <cassert>

constexpr int width = 8;
constexpr int height = 8;
constexpr int Nsquares = 64;
constexpr int max_n_moves = 48;
constexpr int max_depth = 128;

enum class Player {
    White,
    Black
};
enum class Cell {
    Empty,
    White,
    Black,
    Boundary
};
struct Square {
    int col;
    int row;
};
struct Move {
    Square from;
    Square to;
};
constexpr Move Move_None { Square { -1, -1 }, Square { -1, -1 } };

constexpr Player operator!(Player c) {
    return c == Player::White ? Player::Black : Player::White;
}
constexpr Square operator+(const Square& a, const Square& b) {
    return { a.col + b.col, a.row + b.row };
}
inline Square& operator+=(Square& s, const Square& o) {
    s.col += o.col;
    s.row += o.row;
    return s;
}
constexpr bool operator==(const Square& a, const Square& b)
{
    return a.col == b.col && a.row == b.row;
}
constexpr bool operator!=(const Square& a, const Square& b) {
    return a.col != b.col || a.row != b.row;
}
constexpr bool operator==(const Move& a, const Move& b)
{
    return a.from == b.from && a.to == b.to;
}
constexpr bool operator!=(const Move& a, const Move& b) {
    return a.from != b.from || a.to != b.to;
}
constexpr Square relative_square(Player p, const Square& s) {
    return { s.col, p == Player::Black ? 7 - s.row : s.row };
}
constexpr int relative_row(Player p, int row) {
    return p == Player::Black ? 7 - row : row;
}
template<typename T>
constexpr int relative_score(Player p, T score) {
    return p == Player::Black ? -score : score;
}
constexpr Cell cell_of(Player player) {
    return player == Player::White ? Cell::White : Cell::Black;
}
constexpr Player player_of(Cell cell) {
    assert(cell == Cell::White || cell == Cell::Black);
    return cell == Cell::White ? Player::White : Player::Black;
}
constexpr Square square_at(int x, int y) {
    return { x, y };
}
constexpr Move move_fromto(const Square& from, const Square& to) {
    return { from, to };
}


#endif
