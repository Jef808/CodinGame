#ifndef TMVIEW_H_
#define TMVIEW_H_

#include <SFML/Graphics.hpp>

class TileMap : public sf::Drawable {
public:

    bool load(std::string_view tileset, sf::Vector2u tile_size, const int* tiles, size_t width, size_t height)
    {
        if (!m_tileset.loadFromFile(tileset.data()))
            return false;

        m_vertices.setPrimitiveType(sf::Quads);
        m_vertices.resize(width * height * 4);

        for (int i=0; i<height; ++i)
            for (int j=0; j<width; ++j)
            {
                int tile_number = tiles[i * width + j];

                // the position of the tile in the png texture file
                int tx = tile_number % (m_tileset.getSize().x / tile_size.x);
                int ty = tile_number / (m_tileset.getSize().x / tile_size.x);

                sf::Vertex* quad = &m_vertices[(i * width + j) * 4];

                // the position at which the quad is to be drawn,
                // indexing clockwise from the upper left vertex
                quad[0].position = sf::Vector2f(j *       tile_size.x, i *       tile_size.y);
                quad[1].position = sf::Vector2f((j + 1) * tile_size.x, i *       tile_size.y);
                quad[2].position = sf::Vector2f((j + 1) * tile_size.x, (i + 1) * tile_size.y);
                quad[3].position = sf::Vector2f(j *       tile_size.x, (i + 1) * tile_size.y);

                // the corresponding position in the png texture file
                quad[0].texCoords = sf::Vector2f(tx *       tile_size.x, ty *       tile_size.y);
                quad[1].texCoords = sf::Vector2f((tx + 1) * tile_size.x, ty *       tile_size.y);
                quad[2].texCoords = sf::Vector2f((tx + 1) * tile_size.x, (ty + 1) * tile_size.y);
                quad[3].texCoords = sf::Vector2f(tx *       tile_size.x, (ty + 1) * tile_size.y);
            }

        return true;
    }

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.texture = &m_tileset;

        target.draw(m_vertices, states);
    }

private:

    sf::VertexArray m_vertices;
    sf::Texture m_tileset;
};

#endif // DPVIEW_H_
