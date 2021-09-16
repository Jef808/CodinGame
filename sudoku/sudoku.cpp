#include <algorithm>
#include <array>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Sudoku {
public:
    using Move = std::pair<int, int>; // { ndx, digit }

    Sudoku()
        : m_board {}
    {
    }
    /**
     * Apply a move (mutably)
     */
    void apply(const Move&);
    /**
     * Undo a move (mutably)
     */
    void undo(const Move&);
    /**
     * Find any hole or return false for second parameter
     */
    std::pair<int, bool> get_hole() const;
    /**
     * Given a hole, return the list of digits that are playable
     */
    std::array<int, 9> candidates(int row, int col) const;

    void input(std::istream&);
    void output(std::ostream&) const;

private:
    std::array<int, 81> m_board;
};

class Solver {
public:
    using Move = Sudoku::Move;

    Solver(Sudoku& _board)
        : board { _board }
        , valid_moves {}
    {
    }
    /**
     * The main backtracking algorithm.
     */
    bool solve();
    /**
     * Return the winning board if any (otherwise the Sudoku::null() object)
     */
    std::pair<Sudoku, bool> final_board() const;
    /**
     * Pop the top of both our stacks.
     */
    void backtrack();

private:
    Sudoku& board;
    std::deque<Move> edges;
    std::deque<std::array<Move, 9>> valid_moves;
    bool m_winner { false };

    /**
     * Apply the move and push it on the dfs stack.
     */
    void visit(const Move& m);
    /**
     * Compute the candidate moves for the given hole
     * and push them on the valid_moves stack.
     */
    const std::array<Move, 9>& push_next_moves(int hole_ndx);
};

struct Indexer {
    Indexer() = default;

    constexpr static int Index(const int row, const int col)
    {
        return col + row * 9;
    }
    constexpr static int toRow(const int ndx)
    {
        return ndx / 9;
    }
    constexpr static int toCol(const int ndx)
    {
        return ndx % 9;
    }
    constexpr static std::array<int, 9> Row(const int row)
    {
        std::array<int, 9> ret {};
        for (int i = 0; i < 9; ++i)
            ret[i] = Index(row, i);
        std::sort(ret.begin(), ret.end());
        return ret;
    }
    constexpr static std::array<int, 9> Col(const int col)
    {
        std::array<int, 9> ret {};
        for (int j = 0; j < 9; ++j)
            ret[j] = Index(j, col);
        std::sort(ret.begin(), ret.end());
        return ret;
    }
    constexpr static std::array<int, 9> Block(const int ndx)
    {
        std::array<int, 9> ret {};
        const int r = 3 * (toRow(ndx) / 3);
        const int c = 3 * (toCol(ndx) / 3);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) {
                ret[j + 3 * i] = Index((i + r), (j + c));
            }
        std::sort(ret.begin(), ret.end());
        return ret;
    }
    /** All the indices that are 'in contact' with a given square. */
    constexpr static std::array<int, 21> Adjacents(const int row, const int col)
    {
        std::array<int, 21> ret {};
        std::array<int, 9> R = Row(row);
        std::array<int, 9> C = Col(col);
        std::array<int, 9> B = Block(Index(row, col));

        std::set_difference(R.begin(), R.end(), B.begin(), B.end(), ret.begin());
        std::set_difference(C.begin(), C.end(), B.begin(), B.end(), ret.begin() + 6);
        std::copy(B.begin(), B.end(), ret.begin() + 12);

        return ret;
    }
    static constexpr std::array<int, 9> OneToNine { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
};

/** Return the valid candidates at an empty square. */
std::array<int, 9> Sudoku::candidates(int row, int col) const
{
    std::array<int, 9> ret {};
    std::array<int, 21> _ret {};
    std::array<int, 21> A = Indexer::Adjacents(row, col);
    std::transform(A.begin(), A.end(), _ret.begin(), [&](const int n) {
        return m_board[n];
    });
    std::sort(_ret.begin(), _ret.end());
    std::set_difference(Indexer::OneToNine.begin(), Indexer::OneToNine.end(), _ret.begin(), _ret.end(), ret.begin());
    return ret;
}

std::pair<int, bool> Sudoku::get_hole() const
{
    auto it = std::find(m_board.begin(), m_board.end(), 0);
    int ndx = it == m_board.end() ? -1 : std::distance(m_board.begin(), it);
    return std::make_pair(ndx, ndx == -1);
}

inline void Sudoku::apply(const Move& m)
{
    m_board[m.first] = m.second;
}

inline void Sudoku::undo(const Move& m)
{
    m_board[m.first] = 0;
}

inline void Solver::backtrack()
{
    board.undo(edges.back());
    edges.pop_back();
    valid_moves.pop_back();
}

inline void Solver::visit(const Move& m)
{
    board.apply(m);
    edges.push_back(m);
}

const std::array<Solver::Move, 9>& Solver::push_next_moves(int hole_ndx)
{
    auto indices = board.candidates(Indexer::toRow(hole_ndx), Indexer::toCol(hole_ndx));
    auto& moves = valid_moves.emplace_back(std::array<Solver::Move, 9> {}); //{std::pair<int, int>{0, 0}});
    std::transform(indices.begin(), indices.end(), moves.begin(), [h = hole_ndx](const auto n) {
        return Move { h, n };
    });
    return moves;
}

inline bool is_zero(const Solver::Move& m)
{
    return m.second == 0;
}

/**
 * Use a recursive backtracking algorithm, but with minimal
 * memory footprint.
 */
bool Solver::solve()
{
    int hole_ndx;
    std::tie(hole_ndx, m_winner) = board.get_hole();

    if (m_winner)
        return true;

    for (const auto& move : push_next_moves(hole_ndx)) {
        if (is_zero(move))
            continue;

        visit(move);
        if (solve())
            return m_winner = true;
    }

    // Getting here means there was no viable candidate for that hole.
    backtrack();
    return false;
}

/**
 * To be called after Solver::solve()
 */
std::pair<Sudoku, bool> Solver::final_board() const
{
    return std::make_pair(board, m_winner);
}

void Sudoku::input(std::istream& _in)
{
    std::string buf;
    for (int i = 0; i < 9; ++i) {
    std:
        getline(_in, buf);
        for (int j = 0; j < 9; ++j) {
            m_board[j + i * 9] = buf[j] - 48;
        }
    }
}

void Sudoku::output(std::ostream& _out) const
{
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            int d = m_board[j + i * 9];
            if (d == 0) {
                _out << "\e[1;34m";
            }
            _out << d
                 << "\e[0m";
        }
        _out << '\n';
    }
    _out << std::endl;
}

int main(int argc, const char* argv[])
{
    if (argc == 1) {
        std::cerr << "Need input file!" << std::endl;
        return EXIT_FAILURE;
    }

    const char* fn = argv[1];
    std::ifstream ifs { fn };
    if (!ifs) {
        std::cerr << "Could not open file" << std::endl;
        return EXIT_FAILURE;
    }

    Sudoku sudoku {};
    sudoku.input(ifs);

    Solver solver { sudoku };
    solver.solve();

    auto [final_board, winner] = solver.final_board();

    if (winner) {
        std::cout << "Found winner!" << std::endl;
        final_board.output(std::cout);
    } else {
        std::cout << "No solution" << std::endl;
    }

    return EXIT_SUCCESS;
}
