#undef _GLIBCXX_DEBUG // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

#pragma GCC target("bmi,bmi2,lzcnt,popcnt") // bit manipulation
#pragma GCC target("movbe") // byte swap
#pragma GCC target("aes,pclmul,rdrnd") // encryption
#pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

constexpr int width = 8;
constexpr int height = 8;
constexpr int max_n_moves = 48;
constexpr int max_depth = 128;

enum class Player { White = 0,
    Black = 1 };

enum class Cell { Empty,
    White,
    Black,
    Boundary };

struct Square {
    int col;
    int row;
};

struct ExtSquare {
    int col;
    int row;
    bool valid;
    ExtSquare() = default;
    explicit ExtSquare(Square sq)
        : col{sq.col}, row{sq.row}, valid{ true }
    {}
};

struct Move {
    Square from;
    Square to;
};

constexpr Move Move_None { Square { -1, -1 }, Square { -1, -1 } };

std::ostream& operator<<(std::ostream& out, const Cell cell);
std::ostream& operator<<(std::ostream& out, const Move move);

Player operator!(const Player player)
{
    return player == Player::White ? Player::Black : Player::White;
}

Cell operator!(const Cell cell)
{
    return cell == Cell::White ? Cell::Black : Cell::White;
}
constexpr Cell cell_of(Player player)
{
    return player == Player::White ? Cell::White : Cell::Black;
}
constexpr Square square_at(int x, int y) {
    return { x, y };
}
Square operator+(const Square& square, const Square& ds)
{
    return { square.col + ds.col, square.row + ds.row };
}
Square& operator+=(Square& square, const Square& ds)
{
    return square = square + ds;
}
bool operator==(const Square& a, const Square& b)
{
    return a.col == b.col && a.row == b.row;
}
bool operator==(const Move& a, const Move& b)
{
    return a.from == b.from && a.to == b.to;
}

struct StateInfo {
    StateInfo* prev;
    Move move;
    bool is_capture;
    int depth;
};

StateInfo root_state { nullptr, Move_None, false, 0 };

/**
* The square a1 corresponds to (0, 0), a8 to (0, 8) etc...
*/
class Game {

public:
    using valid_move_iterator = std::vector<Move>::const_iterator;
    using valid_move_range = std::pair<valid_move_iterator, valid_move_iterator>;

    Game()
    {
        m_grid.fill(Cell::Empty);
        for (int y = 0; y < height + 2; ++y) {
            m_grid[y * (width + 2)] = m_grid[(y + 1) * (width + 2) - 1] = Cell::Boundary;
        }
        for (int x = 1; x < width + 1; ++x) {
            m_grid[x] = m_grid[x + (height + 2 - 1) * (width + 2)] = Cell::Boundary;
        }
        int n = 0;
        for (int y : { white_row, white_row + 1 }) {
            for (int x = 0; x < 8; ++x) {
                cell_at(x, y) = Cell::White;
                white_pieces[n] = ExtSquare{square_at(x, y)};
                ++n;
            }
        }
        n = 0;
        for (int y : { black_row, black_row - 1 }) {
            for (int x = 0; x < 8; ++x) {
                cell_at(x, y) = Cell::Black;
                black_pieces[n] = ExtSquare{square_at(x, y)};
                ++n;
            }
        }
        n_turns = 0;
        m_player_to_move = Player::White;
    }

    void init(std::istream& _in)
    {
        m_valid_moves.reserve(12 * 6);
        m_buf.clear();

        std::getline(_in, m_buf);
        if (m_buf[0] == 'N') {
            m_player = Player::White;
        } else {
            m_player = Player::Black;
            Move move = get_move(m_buf);
            apply(move, root_state);
        }
        _in >> n_legal_moves;
        _in.ignore();

        for (int i = 0; i < n_legal_moves; ++i) {
            std::getline(_in, m_buf);
            m_valid_moves.push_back(get_move(m_buf));
        }
    }

    void turn_init(std::istream& _in, StateInfo& st)
    {
        m_valid_moves.clear();
        m_buf.clear();
        std::getline(_in, m_buf);
        Move move = get_move(m_buf);

        apply(move, st);

        _in >> n_legal_moves;
        _in.ignore();

        for (int i = 0; i < n_legal_moves; ++i) {
            std::getline(_in, m_buf);
            m_valid_moves.push_back(get_move(m_buf));
        }
    }

    void faster_turn_init(std::istream& _in, StateInfo& st)
    {
        std::getline(_in, m_buf);
        Move move = get_move(m_buf);
        apply(move, st);
        _in >> n_legal_moves;
        _in.ignore();

        for (int i=0; i<n_legal_moves; ++i) {
            std::getline(_in, m_buf);
        }
    }

    void show(std::ostream& out)
    {
        bool reverse_view = m_player == Player::White;

        for (int row = 0; row < height; ++row) {
            out << (reverse_view ? 8 - row : row + 1) << "  ";
            for (int col = 0; col < width; ++col) {
                out << cell_at(col, reverse_view ? row_reversed(row) : row);
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
    }

    valid_move_range valid_moves() const
    {
        return std::make_pair(m_valid_moves.begin(), m_valid_moves.end());
    }

    /// Return true if game is won from the point of view of the last player that moved
    bool is_won()
    {
        Player last_player_moved = !player_to_move();
        int row = last_player_moved == Player::White ? white_row : black_row;
        for (int col = 0; col < width; ++col) {
            if (cell_at(col, row) == cell_of(player_to_move()))
                return false;
        }
        return true;
    }

    void apply(const Move& move, StateInfo& st)
    {

        assert(cell_at(move.from.col, move.from.row) == cell_of(player_to_move()));

        Cell cell = Cell::Empty;

        st.is_capture = is_capture(move);

        auto& pieces = player_to_move() == Player::White ? white_pieces : black_pieces;
        auto piece = std::find_if(pieces.begin(), pieces.end(), [&move](const auto& s){
            return s.valid && s.col == move.from.col && s.row == move.from.row;
        });
        piece->col = move.to.col;
        piece->row = move.to.row;

        if (st.is_capture) {
            auto& opp_pieces = player_to_move() == Player::White ? black_pieces : white_pieces;
            auto opp_piece = std::find_if(opp_pieces.begin(), opp_pieces.end(), [&move](const auto& s){
                return s.valid && s.col == move.to.col && s.row == move.to.row;
            });
            opp_piece->valid = false;
        }

        std::swap(cell, cell_at(move.from.col, move.from.row));
        std::swap(cell, cell_at(move.to.col, move.to.row));

        assert(cell_at(move.to.col, move.to.row) == cell_of(player_to_move()));

        st.move = move;

        st.prev = this->st;
        this->st = &st;

        ++n_turns;
        m_player_to_move = !m_player_to_move;
    }

    void undo(const Move& move)
    {
        assert(cell_at(move.to.col, move.to.row) == !cell_of(player_to_move()));
        assert(this->st->move == move);

        /// If the move we are undoing was a capture, the color was the current player's
        Cell cell = st->is_capture ? cell_of(player_to_move()) : Cell::Empty;

        auto& opp_pieces = player_to_move() == Player::White ? black_pieces : white_pieces;
        auto opp_piece = std::find_if(opp_pieces.begin(), opp_pieces.end(), [&move](const auto& s){
            return s.valid && s.col == move.to.col && s.row == move.to.row;
        });
        opp_piece->col = move.from.col;
        opp_piece->row = move.from.row;

        if (st->is_capture) {
            auto& pieces = player_to_move() == Player::White ? white_pieces : black_pieces;
            auto piece = std::find_if(pieces.begin(), pieces.end(), [&move](const auto& s){
                return !s.valid && s.col == move.to.col && s.row == move.to.row;
            });
            piece->valid = true;
        }

        std::swap(cell, cell_at(move.to.col, move.to.row));
        std::swap(cell, cell_at(move.from.col, move.from.row));

        assert(cell_at(move.from.col, move.from.row) == !cell_of(player_to_move()));

        --n_turns;
        st = st->prev;

        m_player_to_move = !m_player_to_move;
    }

    bool is_capture(const Move& move) const
    {
        return cell_at(move.to) == !cell_of(player_to_move());
    }

    void compute_valid_moves()
    {
        const auto& _offsets = offsets[player_to_move() == Player::White ? 0 : 1];
        m_valid_moves.clear();
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
                    if (to_c == Cell::Empty || to_c == !cell_of(player_to_move())) {
                        out = { Square { col, row }, Square { col + ds.col, row + ds.row } };
                    }
                }
            }
        }
    }

    void compute_valid_moves_faster()
    {
        m_valid_moves.clear();
        auto out = std::back_inserter(m_valid_moves);
        const auto& _offsets = offsets[player_to_move() == Player::White ? 0 : 1];
        const auto& pieces = player_to_move() == Player::White ? white_pieces : black_pieces;
        for (const auto& piece : pieces) {
            if (!piece.valid)
                continue;
            const auto& ds = _offsets[1];
            if (cell_at(piece.col + ds.col, piece.row + ds.row) == Cell::Empty)
                out = { Square { piece.col, piece.row }, Square { piece.col + ds.col, piece.row + ds.row } };
            for (auto i : { 0, 2 }) {
                const auto& ds = _offsets[i];
                Cell cell = cell_at(piece.col + ds.col, piece.row + ds.row);
                if (cell == Cell::Empty || cell == !cell_of(player_to_move()))
                    out = { Square { piece.col, piece.row }, Square { piece.col + ds.col, piece.row + ds.row } };
            }
        }
    }

    Cell cell_at(int x, int y) const
    {
        return m_grid[(x + 1) + (y + 1) * (width + 2)];
    }

    Cell cell_at(const Square& sq) const
    {
        return m_grid[(sq.col + 1) + (sq.row + 1) * (width + 2)];
    }

    int index_of(const Square& sq) { return sq.col + sq.row * (width); }

    Player player_to_move() const
    {
        return m_player_to_move;
    }

    const std::string_view make_move(const Move& move)
    {
        m_buf.clear();
        auto out = std::back_inserter(m_buf);

        out = move.from.col + 97;
        out = move.from.row + 49;
        out = move.to.col + 97;
        out = move.to.row + 49;

        return m_buf;
    }

private:
    std::array<Cell, (width + 2) * (height + 2)> m_grid;
    std::string m_buf;
    std::array<ExtSquare, 16> white_pieces;
    std::array<ExtSquare, 16> black_pieces;
    Player m_player;
    Player m_player_to_move;
    const int white_row { 0 };
    const int black_row { 7 };
    int n_legal_moves;
    std::vector<Move> m_valid_moves;
    int n_turns;
    Square offsets[2][3] {
        { { -1, 1 }, { 0, 1 }, { 1, 1 } }, // Moves for white
        { { -1, -1 }, { 0, -1 }, { 1, -1 } } // Moves for black
    };
    StateInfo* st;

    Cell& cell_at(int x, int y)
    {
        return m_grid[(x + 1) + (y + 1) * (width + 2)];
    }

    int row_reversed(int row)
    {
        return height - 1 - row;
    }

    Move get_move(const std::string& s)
    {
        Move ret {};
        char col = s[0];
        char row = s[1];
        ret.from = { col - 97, row - 49 };
        col = s[2], row = s[3];
        ret.to = { col - 97, row - 49 };

        return ret;
    }
};

class Agent {
public:
    Agent(Game& _game)
        : game(_game)
    {
        move_buf.reserve(max_n_moves);
        std::fill_n(&states[0], max_depth, StateInfo{});
    }

    Move simple_best_move()
    {
        auto [beg, end] = game.valid_moves();
        return *std::max_element(beg, end, MoveCmp { game });
    }

    Move best_move(int s_depth, int s_width)
    {
        int alpha = std::numeric_limits<int>::min();
        int beta = -alpha;

        //game.compute_valid_moves();
        game.compute_valid_moves_faster();
        auto [beg, end] = game.valid_moves();
        std::vector<Move> moves { beg, end };

        StateInfo* st = &states[1];
        st->depth = 0;
        int best_score = alpha;
        Move best_move;

        for (const auto& move : moves) {
            (st+1)->depth = st->depth + 1;
            game.apply(move, *st++);
            int score = eval_minimax(alpha, beta, s_depth, s_width, st, true);
            game.undo(move);

            if (score > best_score) {
                best_score = score;
                best_move = move;
            }
        }

        std::cerr << "Performed " << n_evals << " calls to the eval method" << std::endl;

        return best_move;
    }

    int eval_minimax(int alpha, int beta, int s_depth, int s_width, StateInfo* st, bool maximizing = true)
    {
        ++n_evals;

        if (s_depth == 0) {
            return (1 - 2 * !maximizing) * MoveCmp { game }.score_move((st - 1)->move);
        }

        /// The game is won for current player
        if (game.is_won()) {
            return (1 - 2 * !maximizing) * 5000; // Could return 1000 here, but always change the sign before returning from intermediate states
        }

        std::array<Move, max_n_moves> moves;
        moves.fill(Move_None);
        move_buf.clear();

        // Only use the buffer temporarily, as it may be invalidated in higher stack frames
        //game.compute_valid_moves();
        game.compute_valid_moves_faster();
        auto [beg, end] = game.valid_moves();
        std::copy(beg, end, std::back_inserter(move_buf));
        std::sort(move_buf.begin(), move_buf.end(), MoveCmp(game));
        const int n_actions = move_buf.size() > s_width ? s_width : move_buf.size();
        for (int i = 0; i < n_actions; ++i) {
            moves[i] = move_buf[i];
        }

        int best_score = (1 - 2 * maximizing) * std::numeric_limits<int>::max();

        // The main loop
        if (maximizing) {
            for (auto it = moves.begin(); it != moves.begin() + n_actions; ++it) {
                (st+1)->depth = st->depth + 1;
                game.apply(*it, *st++);

                int score = eval_minimax(alpha, beta, s_depth - 1, s_width, st, !maximizing);

                game.undo(*it);
                --st;

                if (score > best_score) {
                    best_score = score;
                    if (best_score >= beta) // beta cut-off indicates this line is suboptimal for opponent
                        break;
                    if (best_score > alpha)
                        alpha = best_score;
                }
            }
        }
        // minmizing
        else {
            for (auto it = moves.begin(); it != moves.begin() + n_actions; ++it) {

                (st+1)->depth = st->depth + 1;
                game.apply(*it, *st++);

                int score = eval_minimax(alpha, beta, s_depth - 1, s_width, st, !maximizing);

                game.undo(*it);
                --st;

                if (score < best_score) {
                    best_score = score;
                    if (best_score <= alpha) // alpha cut-off indicates a better line exists for the maximzing player
                        break;
                    if (best_score < beta)
                        beta = best_score;
                }
            }
        }

        return best_score;
    }

private:
    Game& game;
    std::vector<Move> move_buf;
    StateInfo states[max_depth];
    int n_evals = 0;

    struct MoveCmp {

        MoveCmp(const Game& g)
            : game(g)
        {
        }

        bool operator()(const Move& a, const Move& b)
        {
            return score_move(a) < score_move(b);
        }

        int score_move(const Move& move) const
        {
            int score = 0;

            // Winning move
            if (move.to.row == 0 || move.to.row == 7) {
                score = 5000;
            }
            if (game.is_capture(move)) {
                // Move that prevents a loss. Everything else should be below 500 to never prevent this
                if (move.from.row == 0 || move.from.row == 7) {
                    score = 500;
                }
                // Pretty much prevents a loss too
                else if (move.from.row == 1 || move.from.row == 6) {
                    score = 200;
                }
                // Give 50 point for captures in general
                else
                    score = 50;
            }

            bool white = move.from.row < move.to.row;
            bool free_pawn = true;
            if (white) {
            for (int i = move.to.row + 1; i < 8; ++i) {
                if (game.cell_at(move.to.col, i) != Cell::Empty) {
                    free_pawn = false;
                    break;
                }
            }
            } else {
                for (int i = move.to.row -1; i > -1; ++i) {
                if (game.cell_at(move.to.col, i) != Cell::Empty) {
                    free_pawn = false;
                    break;
                }
            }
            }

            if (free_pawn) {
                score = 2000;
            }

            // Black
            if (move.from.row > move.to.row) {
                score = (8 - move.to.row) * 100;
            } else if (move.from.row < move.to.row) {
                score = move.to.row * 100;
            }

            return score;
        }

        const Game& game;
    };
};

std::ostream& operator<<(std::ostream& out, const Move move)
{
    return out << "From: (" << move.from.col
               << ", " << move.from.row << ')'
               << " To: (" << move.to.col
               << ", " << move.to.row << ')' << std::endl;
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

int main(int argc, char* argv[])
{
    std::string buf;

    Game game;
    game.init(std::cin);

    Agent agent(game);
    int s_depth = 8;
    int s_width = 32;

    StateInfo st;
    st.depth = 0;


    while (true) {

        auto tik = std::chrono::steady_clock::now();

        Move move = agent.best_move(s_depth, s_width);

        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tik).count();
        std::cerr << "Time taken: " << time << "ms." << std::endl;

        std::cout << game.make_move(move) << std::endl;

        game.apply(move, st);

        game.show(std::cerr);

        game.faster_turn_init(std::cin, st);
    }

    return EXIT_SUCCESS;
}
