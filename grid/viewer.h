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


template<typename TileEnum>
class Viewer : public sf::Drawable {
public:
    /// Up: (0, 0) = bottom left corner, Down: (0, 0) = upper left corner
    enum class YDirection { Up, Down };

    Viewer() = default;

    bool init(const std::string& texture_file, int width, int height, TileSize tile_size) {
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

        /// Set the dimensions of the grid (in number of tiles)
        this->width = width;
        this->height = height;

        /// Set the dimensions of the tile (inside the texture)
        n_tiles_x = m_tileset.getSize().x / this->tile_size;
        n_tiles_y = m_tileset.getSize().y / this->tile_size;

        /// Allocate memory for the quads
        m_vertices.setPrimitiveType(sf::Quads);
        m_vertices.resize(width * height * 4);

        // set the drawing position of the quads
        for (int i = 0; i < height; ++i)
            for (int j = 0; j < width; ++j) {

                sf::Vertex* quad = &m_vertices[(i * width + j) * 4];

                // the position at which the quad is to be drawn,
                // indexing clockwise from the upper left vertex
                quad[0].position = sf::Vector2f(j * this->tile_size, i * this->tile_size);
                quad[1].position = sf::Vector2f((j + 1) * this->tile_size, i * this->tile_size);
                quad[2].position = sf::Vector2f((j + 1) * this->tile_size, (i + 1) * this->tile_size);
                quad[3].position = sf::Vector2f(j * this->tile_size, (i + 1) * this->tile_size);
            }

        /// Allocate memory for the buffers
        m_background.clear();
        m_background.resize(width * height);

        /// Setup parameters for the messages
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

    void show_status(std::ostream& out) {
        out << "Number of tiles: " << m_tilemap.size()
            << "\nBackgroud size: " << m_background.size()
            << "\ntile_size: " << tile_size
            << "\nn_tiles_x " << n_tiles_x
            << "\nn_tiles_y " << n_tiles_y
            << std::endl;
    }

    void set_tilepos_bg(TileEnum tile, int x, int y) {
        m_background[index(x, y)] = get(tile);
    }

    void set_tilepos_fg(TileEnum tile, int x, int y) {
        auto ndx = index_out(x, y);
        auto tile_num = get(tile);
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

    void reset() {
        for (int x = 0; x < width; ++x)
            for (int y = 0; y < height; ++y) {
                auto ndx = index_out(x, y);
                auto ndx_out = index_out(x, y);
                auto tile_num = m_background[ndx];
                sf::Vertex* quad = &m_vertices[ndx_out * 4];

                // the row/column coordinates of the target texture tile
                int tx = tile_num % n_tiles_x;
                int ty = tile_num / n_tiles_y;

                std::cout << "Setting pos " << x << ' ' << y
                    << " to " << tx * tile_size << ' ' << ty * tile_size
                    << std::endl;

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
    int tile_size;
    int width;
    int height;
    int n_tiles_x;
    int n_tiles_y;
    YDirection y_direction{ YDirection::Up };

    int get(TileEnum tile) {
        int tile_num = std::lower_bound(m_tilemap.begin(), m_tilemap.end(), tile, CmpTileMap)->second;
        std::cout << "tile " << tile_num << std::endl;
        return tile_num;
    }

    size_t index(int x, int y) {
        return x + width * y;
    }

    size_t index_out(int x, int y) {
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
