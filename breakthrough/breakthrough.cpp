#define RUNNING_OFFLINE 1

// #undef _GLIBCXX_DEBUG // disable run-time bound checking, etc
// #pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

// #pragma GCC target("bmi,bmi2,lzcnt,popcnt") // bit manipulation
// #pragma GCC target("movbe") // byte swap
// #pragma GCC target("aes,pclmul,rdrnd") // encryption
// #pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include "helpers.h"

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
        : col { sq.col }
        , row { sq.row }
        , valid { true }
    {
    }
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
constexpr Square square_at(int x, int y)
{
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

    void init()
    {
        m_valid_moves.reserve(12 * 6);
        m_buf.clear();

        m_player = Player::White;
        n_legal_moves = 22;
    }

    void turn_init(StateInfo& st)
    {
        compute_valid_moves();
        apply(m_valid_moves[rand() % n_legal_moves], st);
        compute_valid_moves();
        n_legal_moves = m_valid_moves.size();
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
            //m_valid_moves.push_back(get_move(m_buf));
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
            //m_valid_moves.push_back(get_move(m_buf));
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

    /// Return true if either side won
    bool is_terminal()
    {
        auto* itw = &cell_at(0, 0), *endw = &cell_at(7, 0);
        auto* itb = &cell_at(0, height - 1), *endb = &cell_at(7, height - 1);
        for (; itw != endw; ++itw, ++itb) {
            if (*itw == Cell::Black || *itb == Cell::White)
                return true;
        }
        return false;

    }
    /// Return true if game is won from the point of view of the last player that moved
    bool is_won()
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

    void apply(const Move& move, StateInfo& st)
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

    void undo(const Move& move)
    {
        assert(!(move == Move_None));
        assert(cell_at(move.to.col, move.to.row) == !cell_of(player_to_move()));
        assert(this->st->move == move);

        /// If the move we are undoing was a capture, the color was the current player's
        Cell cell = st->is_capture ? cell_of(player_to_move()) : Cell::Empty;

        std::swap(cell, cell_at(move.to.col, move.to.row));
        std::swap(cell, cell_at(move.from.col, move.from.row));

        assert(cell_at(move.from.col, move.from.row) == !cell_of(player_to_move()));

        --n_turns;
        st = st->prev;

        m_player_to_move = !m_player_to_move;
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

    bool is_capture(const Move& move) const
    {
        return cell_at(move.to) == !cell_of(player_to_move());
    }

    int turn_number() const {
        return n_turns;
    }

    int n_pieces_left() const {
        return std::count_if(m_grid.begin(), m_grid.end(), [](const auto c){
            return c != Cell::Empty && c != Cell::Boundary;
        });
    }
    valid_move_range valid_moves() const
    {
        return std::make_pair(m_valid_moves.begin(), m_valid_moves.end());
    }
    Cell cell_at(int x, int y) const
    {
        return m_grid[(x + 1) + (y + 1) * (width + 2)];
    }

    Cell cell_at(const Square& sq) const
    {
        return m_grid[(sq.col + 1) + (sq.row + 1) * (width + 2)];
    }

    int index_of(const Square& sq)
    {
        return sq.col + sq.row * (width);
    }

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

    struct ExtMove {
        Move move;
        int value;
        explicit ExtMove(Move _move)
            : move { _move }
            , value { 0 }
        {
        }
        bool operator<(const ExtMove& move) const {
            return this->value > move.value;
        }
        bool operator==(const Move& move) const
        {
            return move == this->move;
        }
        bool operator!=(const ExtMove& emove) const
        {
            return !(this->move == emove.move);
        }
    };

    struct Stack {
        int depth;
        int move_count;
    };

public:
    Agent(Game& _game)
        : game(_game)
    {
        move_buf.reserve(max_n_moves);
    }

    Move simple_best_move()
    {
        game.compute_valid_moves();
        auto [beg, end] = game.valid_moves();
        return *std::max_element(beg, end, [&](const auto& a, const auto& b){
            return score_move(a) < score_move(b);
        });
    }

    Move best_move(int s_width, int s_depth)
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

    int eval_minimax(int alpha, int beta, int s_depth, int s_width, Stack* ss)
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

private:
    Game& game;
    std::vector<ExtMove> root_moves;
    std::vector<Move> move_buf;
    StateInfo states[max_depth];
    int n_evals = 0;

    int score_move(const Move& move) const
    {
        int score = 0;

        // Winning move
        if (move.to.row == 0 || move.to.row == 7) {
            return 10000;
        }

        if (game.is_capture(move)) {
            // NOTE: This move is highly valuable as a move, but if we are trying to
            // evaluate a position statically, it should probably be considered bad
            // to be in that shape in the first place.
            // So this method should be used in the move loop before applying the actions
            // in view of *pruning* them, not to evaluate the state at depth 0
            if (move.from.row == 0 || move.from.row == 7) {
                return 10000 - 1;
            }
            // Pretty much prevents a loss too. It would be better to have some kind of
            // gradient from the outer rows to the center, weighted by the amount of
            // nearby pawns, making it easier to storm through or defend
            else if (move.from.row == 1 || move.from.row == 6) {
                score += 10000 - 2;
            }
            // Give 100 point for captures in general
            else
                score += 100;
        }

        // TODO: Some mechanism to detect passed pawns, and check if
        // it is possible to storm through and win directly.
        bool white = move.from.row < move.to.row;
        bool passed_pawn = true;

        if (white) {

            int offsets[3][2] = {{-1, 1}, {0, 0}, {1, 1}};

            for (int i = move.to.row; i < 8; ++i) {

                for (const auto& dp : offsets) {

                    if (game.cell_at({ move.to.col + dp[0], move.to.row + dp[1] }) != Cell::Empty) {

                        passed_pawn = false;
                        break;
                    }
                }

                if (!passed_pawn) break;
            }

            // This is almost a Win in *distance to the end* turns
            // but there could be a pawn coming from far away diagonally
            // This is probably too much to check for every move of every node,
            // with bitboards it would be fine
            if (passed_pawn)
                score += 5000 - ( 7 - move.from.row );
        }
       else
        {
            int offsets[3][2] = {{-1, -1}, {0, 0}, {1, -1}};

            for (int i = move.to.row; i >= 0; --i) {

                for (const auto& dp : offsets) {

                    if (game.cell_at({ move.to.col + dp[0], move.to.row + dp[1] }) != Cell::Empty) {

                        passed_pawn = false;
                        break;
                    }
                }

                if (!passed_pawn) break;

            }
                if (passed_pawn)
                    score += 5000 - move.from.row;
        }


        // slight bias towards moves closer to the goal
        if (move.from.row > move.to.row) {

            score += (8 - move.to.row) * 20;

        } else if (move.from.row < move.to.row) {

            score += move.to.row * 20;
        }

        return score;

    }
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

#if RUNNING_OFFLINE
    game.init();

#else
    game.init(std::cin);

#endif
    Agent agent(game);
    int s_depth = 6;
    int s_width = 12;

    StateInfo states[max_depth];
    StateInfo* st { &states[0] };
    bool done = false;

    while (!done) {

        auto tik = std::chrono::steady_clock::now();

        Move move = agent.best_move(s_width, s_depth);

        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tik).count();
        std::cerr << "Time taken: " << time << "ms." << std::endl;

        std::cout << game.make_move(move) << std::endl;

        game.apply(move, *st++);

#if RUNNING_OFFLINE

        if (!game.is_terminal());
            game.turn_init(*st++);

        done = game.is_terminal();

#else
        game.turn_init(std::cin, *st++);

#endif
        // game.show(std::cerr);

    }

    return EXIT_SUCCESS;
}
