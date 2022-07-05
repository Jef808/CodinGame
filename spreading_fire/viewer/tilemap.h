#ifndef TILEMAP_H_
#define TILEMAP_H_

#include <SFML/Graphics.hpp>
#include <functional>

#include <iostream>
#include <cassert>

class TileMap
{
public:

    // TileMap(unsigned int width, unsigned int height, sf::Vector2u tile_size)
    //     : m_width{ width }, m_height{ height }, m_tileSize{ tile_size }
    // {}
    // bool load(const std::string& tileset, sf::Vector2u tileSize, unsigned int width, unsigned int height)
    // {
    //     // load the tileset texture
    //     if (!m_tileset.loadFromFile(tileset))
    //         return false;

    //     m_width = width;
    //     m_height = height;
    //     m_tileSize = tileSize;

    //     return true;
    // }

    void init(unsigned int width, unsigned int height, sf::Vector2u tile_size) {
        m_width = width;
        m_height = height;
        m_tileSize = tile_size;
    }


    /**
     * Populates the VertexAray input by connecting the set of coordinates in the
     * tileset png file to the right set of 2d coordinates on the screen.
     */
    void set_tiles(const int* tiles, sf::Texture& texture, sf::VertexArray& vertices)
    {

        // for each quad, assign it both sets of coordinates
        for (unsigned int i = 0; i < m_width; ++i)
            for (unsigned int j = 0; j < m_height; ++j)
            {
                int tileNumber = tiles[i + j * m_width];

                // find its position in the tileset texture
                int tu = tileNumber % (texture.getSize().x / m_tileSize.x);
                int tv = tileNumber / (texture.getSize().x / m_tileSize.x);

                // get a pointer to the current tile's quad
                sf::Vertex* quad = &vertices[(i + j * m_width) * 4];

                // set its 4 corners
                quad[0].position = sf::Vector2f(i * m_tileSize.x, j * m_tileSize.y);
                quad[1].position = sf::Vector2f((i + 1) * m_tileSize.x, j * m_tileSize.y);
                quad[2].position = sf::Vector2f((i + 1) * m_tileSize.x, (j + 1) * m_tileSize.y);
                quad[3].position = sf::Vector2f(i * m_tileSize.x, (j + 1) * m_tileSize.y);

                // set its 4 texture coordinates
                quad[0].texCoords = sf::Vector2f(tu * m_tileSize.x, tv * m_tileSize.y);
                quad[1].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, tv * m_tileSize.y);
                quad[2].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, (tv + 1) * m_tileSize.y);
                quad[3].texCoords = sf::Vector2f(tu * m_tileSize.x, (tv + 1) * m_tileSize.y);
            }
    }


private:

    unsigned int m_width;
    unsigned int m_height;

    sf::Vector2u m_tileSize;
};

#endif // TILEMAP_H_
