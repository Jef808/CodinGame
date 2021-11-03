#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

constexpr int width = 8;
constexpr int height = 8;

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

struct Move {
    Square from;
    Square to;
};

std::ostream& operator<<(std::ostream& out, const Cell cell);

class Game {

public:
    Game()
    {
        m_grid.fill(Cell::Empty);
        for (int y = 0; y < height + 2; ++y) {
            m_grid[y * (width + 2)] = m_grid[(y + 1) * (width + 2) - 1] = Cell::Boundary;
        }
        for (int x = 1; x < width + 1; ++x) {
            m_grid[x] = m_grid[x + (height + 2 - 1) * (width + 2)] = Cell::Boundary;
        }
        for (int y : { 0, 1 }) {
            for (int x = 0; x < 8; ++x) {
                cell_at(x, y) = Cell::White;
            }
        }
        for (int y : { 6, 7 }) {
            for (int x = 0; x < 8; ++x) {
                cell_at(x, y) = Cell::Black;
            }
        }
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
            apply(move);
        }
        _in >> n_legal_moves;
        _in.ignore();

        for (int i = 0; i < n_legal_moves; ++i) {
            std::getline(_in, m_buf);
            m_valid_moves.push_back(get_move(m_buf));
        }
    }

    void turn_init(std::istream& _in)
    {
        m_valid_moves.clear();
        m_buf.clear();
        std::getline(_in, m_buf);
        Move move = get_move(m_buf);

        std::cerr << "Applying " << m_buf << std::endl;

        apply(move);

        _in >> n_legal_moves;
        _in.ignore();

        for (int i = 0; i < n_legal_moves; ++i) {
            std::getline(_in, m_buf);
            m_valid_moves.push_back(get_move(m_buf));
        }
    }

    void show(std::ostream& out)
    {
        for (int row = 0; row < height; ++row) {
            out << row + 1 << "  ";
            for (int col = 0; col < width; ++col) {
                out << cell_at(col, row_reversed(row));
            }
            out << '\n';
        }
        out << "   ";
        for (int col = 0; col < width; ++col) {
            char c = col + 97;
            out << ' ' << c << ' ';
        }
        out << std::endl;
    }

    const std::vector<Move>& valid_moves() const { return m_valid_moves; }

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

    void apply(const Move& move)
    {
        assert(cell_at(move.from.col, move.from.row) != Cell::Empty);
        assert(cell_at(move.from.col, move.from.row) != Cell::Boundary);

        Cell cell = Cell::Empty;

        std::swap(cell, cell_at(move.from.col, move.from.row));
        std::swap(cell, cell_at(move.to.col, move.to.row));

    }

    Cell cell_at(int x, int y) const
    {
        return m_grid[(x + 1) + (y + 1) * (width + 2)];
    }

    int index_of(const Square& sq) { return sq.col + sq.row * (width); }

private:
    std::array<Cell, (width + 2) * (height + 2)> m_grid;
    std::string m_buf;
    Player m_player;
    int n_legal_moves;
    std::vector<Move> m_valid_moves;

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
        char col = s[0], row = s[1];
        ret.from = { col - 97, row - 49 };
        col = s[2], row = s[3];
        ret.to = { col - 97, row - 49 };

        return ret;
    }
};

class Agent {
};

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
    Game game;
    game.init(std::cin);

    game.show(std::cerr);

    while (true) {
        const auto& moves = game.valid_moves();
        Move move = moves[rand() % moves.size()];
        game.apply(move);
        std::cout << game.make_move(move) << std::endl;
        game.turn_init(std::cin);

        game.show(std::cerr);
    }

    return EXIT_SUCCESS;
}
