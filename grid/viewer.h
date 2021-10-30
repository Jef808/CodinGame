#ifndef DPVIEW_H_
#define DPVIEW_H_

#include <iostream>
#include <string>
#include <type_traits>
#include <map>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>


template<typename Enum>
inline typename std::underlying_type_t<Enum>
to_int(Enum e){ return static_cast<std::underlying_type_t<Enum>>(e); }

namespace sf {
class RenderTarget;
class RenderStates;
}

namespace Grid {

enum class TileSize {
    Small,
    Medium,
    Big
};

/// Up: (0, 0) = bottom left corner, Down: (0, 0) = upper left corner
enum class YDirection {
    Up,
    Down
};

template<typename TileEnum>
class Viewer : public sf::Drawable {
public:

    Viewer() = default;

    bool init(const std::string& texture_file, int width, int height, TileSize tile_size, YDirection y_dir = YDirection::Up) {
        /// Load the texture file
        if (!m_tileset.loadFromFile(texture_file)) {
            std::cerr << "Viewerer: failed to open texture file"
                      << std::endl;
            return false;
        }

        /// Load the font size
        if (!m_font.loadFromFile("resources/DejaVuSans.ttf")) {
            std::cerr << "Viewerer: failed to open texture file"
                      << std::endl;
            return false;
        }

        /// Set the size of the tiles
        switch(tile_size) {
            case TileSize::Small:  this->tile_size = 32;  break;
            case TileSize::Medium: this->tile_size = 64;  break;
            case TileSize::Big:    this->tile_size = 128; break;
        }

        /// The the origin
        this->y_direction = y_dir;

        /// Set the dimensions of the grid (in number of tiles)
        this->width = width;
        this->height = height;

        /// Set the dimensions of the tile (in the texture file)
        n_tiles_x = m_tileset.getSize().x / this->tile_size;
        n_tiles_y = m_tileset.getSize().y / this->tile_size;

        /// Allocate memory for the quads
        m_vertices.setPrimitiveType(sf::Quads);
        m_vertices.resize(width * height * 4);

        /// Allocate memory and initialize the buffers
        m_background.resize(width * height);
        m_foreground.resize(width * height);

        m_background.push_back(-1);
        std::fill_n(&m_background[0], width * height, -1);
        m_foreground.push_back(-1);
        std::fill_n(&m_foreground[0], width * height, -1);

        // set the drawing position of the quads
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x) {

                sf::Vertex* quad = &m_vertices[y_direction == YDirection::Up
                                               ? index_reversed(x, y) * 4
                                               : index(x, y) * 4];

                // indexing clockwise from the upper left vertex
                quad[0].position = sf::Vector2f(x * this->tile_size, y * this->tile_size);
                quad[1].position = sf::Vector2f((x + 1) * this->tile_size, y * this->tile_size);
                quad[2].position = sf::Vector2f((x + 1) * this->tile_size, (y + 1) * this->tile_size);
                quad[3].position = sf::Vector2f(x * this->tile_size, (y + 1) * this->tile_size);
            }

        /// Setup parameters for displaying messages
        m_text.setFont(m_font);
        m_text.setCharacterSize(24);
        m_text.setFillColor(sf::Color::Red);
        m_text.setStyle(sf::Text::Bold);
        m_text.setPosition(8.0, this->tile_size * this->height + 8.0);

        return true;
    }

    void set_orientation(YDirection dir) {
        this->y_direction = dir;
    }

    /// Insert while keeping the tilemap sorted
    void register_tile(TileEnum tile, int index) {
        auto lower_bound = std::lower_bound(m_tilemap.begin(), m_tilemap.end(), tile, CmpTileMap);
        m_tilemap.insert(lower_bound, std::make_pair(tile, index));
    }

    void show_tilemap(std::ostream& out) {
        int c = 0;
        for (const auto& kv : m_tilemap) {
            out << "Tile " << c << ": " << kv.second << std::endl;
            ++c;
        }
    }

    void set_tilepos_bg(TileEnum tile, int x, int y) {
        m_background[index(x, y)] = get(tile);
    }

    void set_tilepos_fg(TileEnum tile, int x, int y) {
        m_foreground[index(x, y)] = get(tile);
    }

    void reset_fg() {
        std::fill_n(m_foreground.begin(), width * height, -1);
    }

    void show_status(std::ostream& out) {
        out << "Number of tiles: " << m_tilemap.size()
            << "\nBackgroud size: " << m_background.size()
            << "\ntile_size: " << tile_size
            << "\nn_tiles_x " << n_tiles_x
            << "\nn_tiles_y " << n_tiles_y
            << std::endl;
    }

    void freeze() {
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x) {
                auto ndx = index(x, y);
                auto tile_num = m_foreground[ndx] != -1
                    ? m_foreground[ndx]
                    : m_background[ndx];

                if (tile_num == -1) {
                    std::cerr << "Viewer: Warning! tile ("
                              << x << ", " << y
                              << ") unset!" << std::endl;
                    continue;
                }

                sf::Vertex* quad = &m_vertices[ndx * 4];

                // the row/column coordinates of the target texture tile
                int tx = tile_num % n_tiles_x;
                int ty = tile_num / n_tiles_x;

                // make the quad point to the corresponding squares in the texture itself
                quad[0].texCoords = sf::Vector2f(tx * tile_size, ty * tile_size);
                quad[1].texCoords = sf::Vector2f((tx + 1) * tile_size, ty * tile_size);
                quad[2].texCoords = sf::Vector2f((tx + 1) * tile_size, (ty + 1) * tile_size);
                quad[3].texCoords = sf::Vector2f(tx * tile_size, (ty + 1) * tile_size);
            }
    }

    void set_message(std::string_view message) {
        m_text.setString(message.data());
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.texture = &m_tileset;
        target.draw(m_vertices, states);
        target.draw(m_text);
    }

private:
    sf::VertexArray m_vertices;
    sf::Texture m_tileset;
    sf::Text m_text;
    sf::Font m_font;
    std::vector<std::pair<TileEnum, int>> m_tilemap;
    std::vector<int> m_background;
    std::vector<int> m_foreground;
    int tile_size;
    int width;
    int height;
    int n_tiles_x;
    int n_tiles_y;
    YDirection y_direction{ YDirection::Up };

    int get(TileEnum tile) {
        int tile_num = std::lower_bound(m_tilemap.begin(), m_tilemap.end(), tile, CmpTileMap)->second;
        return tile_num;
    }

    size_t index(int x, int y) {
        return x + width * y;
    }

    size_t index_reversed(int x, int y) {
        return x + width * (height - 1 - y);
    }

    struct {
        bool operator()(const std::pair<TileEnum, int>& a, TileEnum b) const {
            return to_int(a.first) < to_int(b);
        }
        bool operator()(const std::pair<TileEnum, int>& a, const std::pair<TileEnum, int>& b) const {
            return to_int(a.first) < to_int(b.first);
        }
    } CmpTileMap;
};

}  // namespace view

#endif // DPVIEW_H_
