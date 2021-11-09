#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

#include "breakthrough.h"

StateInfo root_state;
int n_legal_moves;
std::vector<Move> m_valid_moves;
std::vector<Square> square_buf;
std::string m_buf;
std::string view_buf;
std::string ssquare_buf;
std::string smove_buf;
Square offsets[2][3] {
    { { -1, 1 }, { 0, 1 }, { 1, 1 } }, // Moves for white
    { { -1, -1 }, { 0, -1 }, { 1, -1 } } // Moves for black
};
constexpr int white_row = 0;
constexpr int black_row = 7;

std::ostream& operator<<(std::ostream& out, const Cell cell);
std::ostream& operator<<(std::ostream& out, const Square square);
std::ostream& operator<<(std::ostream& out, const Move move);

Game::Game() {
    m_grid.fill(Cell::Empty);
    for (int y = 0; y < height + 2; ++y) {
        m_grid[y * (width + 2)] = m_grid[(y + 1) * (width + 2) - 1] = Cell::Boundary;
    }
    for (int x = 1; x < width + 1; ++x) {
        m_grid[x] = m_grid[x + (height + 2 - 1) * (width + 2)] = Cell::Boundary;
    }
    for (int y : { white_row, white_row + 1 }) {
        for (int x = 0; x < 8; ++x) {
            cell_at(x, y) = Cell::White;
        }
    }
    for (int y : { black_row, black_row - 1 }) {
        for (int x = 0; x < 8; ++x) {
            cell_at(x, y) = Cell::Black;
        }
    }
    n_turns = 0;
    m_player_to_move = Player::White;
}

void Game::init(std::istream& is) {
    m_valid_moves.reserve(12 * 6);
    m_buf.clear();

    std::getline(is, m_buf);
    if (m_buf[0] == 'N') {
        m_player = Player::White;
    } else {
        m_player = Player::Black;
        Move move = get_move(m_buf);
        apply(move, root_state);
    }
    is >> n_legal_moves;
    is.ignore();

    for (int i = 0; i < n_legal_moves; ++i) {
        std::getline(is, m_buf);
        m_valid_moves.push_back(get_move(m_buf));
    }
}

void Game::turn_init(std::istream& is, StateInfo& st) {
    m_valid_moves.clear();
    m_buf.clear();
    std::getline(is, m_buf);
    Move move = get_move(m_buf);

    apply(move, st);

    is >> n_legal_moves;
    is.ignore();

    for (int i = 0; i < n_legal_moves; ++i) {
        std::getline(is, m_buf);
        m_valid_moves.push_back(get_move(m_buf));
    }
}

void Game::set_board(std::string_view s, Player to_move) {
    for (int row = height - 1; row >= 0; --row) {
        for (int col = 0; col < width; ++col) {
            Cell& c = cell_at(col, row);
            switch(s[col + (7 - row) * width]) {
                case '.':
                    c = Cell::Empty; break;
                case 'W':
                    c = Cell::White; break;
                case 'B':
                    c = Cell::Black; break;
                default:
                    std::cerr << "Game::set_state: unknown character read: "
                        << s[col + (7 - row) * width] << std::endl;
                    assert(false);
            }
        }
    }
    m_player_to_move = to_move;
}

std::string_view Game::view_square(const Square& sq)
{
    ssquare_buf.clear();
    auto out = std::back_inserter(ssquare_buf);

    out = sq.col + 97;
    out = sq.row + 49;

    return ssquare_buf;
}

std::string_view Game::view_move(const Move& move)
{
    std::stringstream ss;

    ss << view_square(move.from);
    ss << view_square(move.to);

    smove_buf = ss.str();

    return smove_buf;
}

Move Game::get_move(std::string_view s)
{
    Move ret {};
    char col = s[0];
    char row = s[1];
    ret.from = { col - 97, row - 49 };
    col = s[2], row = s[3];
    ret.to = { col - 97, row - 49 };

    return ret;
}

std::string_view Game::view() const
{
    std::ostringstream out;

    out << "       "
        << (m_player_to_move == Player::White ? "WHITE " : "BLACK ")
        << "to play\n";

    for (int row = height - 1; row >= 0; --row) {
        out << row + 1 << "  ";
        for (int col = 0; col < width; ++col) {
            out << cell_at(col, row);
        }
        out << '\n';
    }
    out << "   ";
    for (int col = 0; col < width; ++col) {
        char c = col + 97;
        out << ' ' << c << ' ';
    }
    out << '\n'
        << std::endl;

    view_buf = out.str();

    return view_buf;
}

/// Return true if game is won from the point of view of the last player that moved
bool Game::is_won()
{
    bool turn_white = player_to_move() == Player::White;
    auto* it = turn_white ? &cell_at(0, 0) : &cell_at(0, height - 1);
    auto* end = turn_white ? &cell_at(7, 0) : &cell_at(7, height - 1);
    for (; it != end; ++it) {
        if (*it == cell_of(!player_to_move())) {
            return true;
        }
    }
    return false;
}

void Game::apply(const Move& move, StateInfo& st)
{
    assert(!(move == Move_None));
    assert(cell_at(move.from.col, move.from.row) == cell_of(player_to_move()));

    Cell cell = Cell::Empty;

    st.is_capture = is_capture(move);

    std::swap(cell, cell_at(move.from.col, move.from.row));
    std::swap(cell, cell_at(move.to.col, move.to.row));

    assert(cell_at(move.to.col, move.to.row) == cell_of(player_to_move()));

    st.move = move;

    st.prev = this->st;
    this->st = &st;

    ++n_turns;
    m_player_to_move = !m_player_to_move;
}

void Game::undo(const Move& move)
{
    assert(!(move == Move_None));
    assert(cell_at(move.to.col, move.to.row) == cell_of(!player_to_move()));
    assert(this->st->move == move);

    /// If the move we are undoing was a capture, the color was the current player's
    Cell cell = st->is_capture ? cell_of(player_to_move()) : Cell::Empty;

    std::swap(cell, cell_at(move.to.col, move.to.row));
    std::swap(cell, cell_at(move.from.col, move.from.row));

    assert(cell_at(move.from.col, move.from.row) == cell_of(!player_to_move()));

    --n_turns;
    st = st->prev;

    m_player_to_move = !m_player_to_move;
}

void Game::compute_valid_moves() const
{
    m_valid_moves.clear();
    const auto& _offsets = offsets[player_to_move() == Player::White ? 0 : 1];
    auto out = std::back_inserter(m_valid_moves);

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            Cell from_c = cell_at(col, row);
            if (from_c != cell_of(player_to_move())) {
                continue;
            }
            const auto& ds = _offsets[1];

            /// Can only capture diagonally
            if (cell_at(col + ds.col, row + ds.row) == Cell::Empty)
                out = { Square { col, row }, Square { col + ds.col, row + ds.row } };
            for (auto i : { 0, 2 }) {
                const auto& ds = _offsets[i];
                Cell to_c = cell_at(col + ds.col, row + ds.row);
                if (to_c == Cell::Empty || to_c == cell_of(!player_to_move())) {
                    out = { Square { col, row }, Square { col + ds.col, row + ds.row } };
                }
            }
        }
    }
}

Game::square_range Game::pawns_of(Player p) const {
    square_buf.clear();
    for (int row = 0; row < height; ++row)
        for (int col = 0; col < width; ++col)
            if (cell_at(col, row) == cell_of(p))
                square_buf.push_back({ col, row });
    return std::make_pair(square_buf.begin(), square_buf.end());
}

Game::valid_move_range Game::valid_moves() const {
    return std::make_pair(m_valid_moves.begin(), m_valid_moves.end());
}

bool Game::test_move_gen(bool display) const {
    static std::vector<Move> buf;
    buf = m_valid_moves;

    auto cmp_mv = [](const auto& a, const auto& b) {
        auto cmp_sq = [](const auto& a, const auto& b) {
            return a.col < b.col || a.col == b.col && a.row < b.row;
        };
        return cmp_sq(a.from, b.from) || a.from == b.from && cmp_sq(a.to, b.to);
    };

    std::sort(buf.begin(), buf.end(), cmp_mv);
    compute_valid_moves();
    std::sort(m_valid_moves.begin(), m_valid_moves.end(), cmp_mv);

    auto game_mv = buf.begin();
    auto my_mv = m_valid_moves.begin();

    if (display) {
        std::cerr << "Game moves:\n";
        for (auto it = game_mv; it != buf.end(); ++it)
            std::cerr << Game::view_move(*it) << ' ';
        std::cerr << "\nMy moves:\n";
        for (auto it = my_mv; it != m_valid_moves.end(); ++it)
            std::cerr << Game::view_move(*it) << ' ';
        std::cerr << std::endl;
    }

    for (; game_mv != buf.end() && my_mv != m_valid_moves.end();
         ++game_mv, ++my_mv)
    {
        if (*game_mv != *my_mv) {
            std::cerr << "Error: "
                << Game::view_move(*game_mv)
                << " != "
                << Game::view_move(*my_mv)
                << std::endl;
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out, const Square square)
{
    return out << Game::view_square(square);
}

std::ostream& operator<<(std::ostream& out, const Move move)
{
    return out << Game::view_move(move);
}

std::ostream& operator<<(std::ostream& out, const Cell cell)
{
    switch (cell) {
    case Cell::Empty:
        return out << " . ";
    case Cell::White:
        return out << " W ";
    case Cell::Black:
        return out << " B ";
    default: {
        std::cerr << "invalid Cell sent to output stream" << std::endl;
        assert(false);
    }
    }
}
