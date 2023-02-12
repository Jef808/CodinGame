#ifndef WORLD_H_
#define WORLD_H_

#include <iosfwd>
#include <vector>

struct Tile {
    int x{-1};
    int y{-1};
    int scrap_amount{-1};
    int owner{-1};
    int units{-1};
    bool recycler{false};
    bool can_build{false};
    bool can_spawn{false};
    bool in_range_of_recycler{false};

    std::ostream& dump(std::ostream& ioOut) const;
};

class World {
  public:
    /**
     * The default constructor assigns values to `m_width` and `m_height`
     * from standard input.
     */
    World();

    [[nodiscard]] unsigned long width() const { return m_width; }

    [[nodiscard]] unsigned long height() const { return m_height; }

    [[nodiscard]] Tile tile(unsigned long idx) const { return m_tiles[idx]; }

    [[nodiscard]] Tile tile(int row, int col) const {
        return m_tiles[get_idx(row, col)];
    }

    const Tile& update_tile(int row, int col);

    [[nodiscard]] unsigned long get_idx(int row, int col) const {
        return col + row * m_width;
    }

  private:
    int m_width{};
    int m_height{};
    std::vector<Tile> m_tiles{};
};

#endif // WORLD_H_
