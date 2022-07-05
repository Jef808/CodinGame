#ifndef VIEWER_H_
#define VIEWER_H_

#include "game.h"
#include "tilemap.h"

#include <SFML/Graphics.hpp>

#include <cassert>
#include <algorithm>
#include <deque>
#include <functional>
#include <vector>



class Viewer : public sf::Drawable, public sf::Transformable {

enum Tile {
    Tree = 0,
    Water = 1,
    Fire = 2,
    Burnt = 3,
    Cut = 4,
    Mushroom = 5
};

/**
 * Translation utility that takes game cells objects and
 * picks their corresponding index in the tileset png file.
 */
int tr(const Cell& c) {
    switch(c.status())
    {
        case Cell::Status::OnFire: return Viewer::Tile::Fire;
        case Cell::Status::Burnt: return Viewer::Tile::Burnt;
        case Cell::Status::Cutting:
        case Cell::Status::Cut: return Viewer::Tile::Cut;
        case Cell::Status::NoFire: {
            switch (c.type())
            {
                case Cell::Type::Safe: return Viewer::Tile::Water;
                case Cell::Type::Tree: return Viewer::Tile::Tree;
                case Cell::Type::House: return Viewer::Tile::Mushroom;
            }
        }
        default: assert(false && "Unimplemented Cell type/status combination sent to viewer");
    }

    throw(std::runtime_error("Ugh"));
}

public:

    bool init(const Game& game) {
        m_width = game.width();
        m_height = game.height();
        m_tile_size = sf::Vector2u(64, 64);

        if (!m_tileset.loadFromFile(VIEWER_DATA_DIR "/Tileset_384x64.png")) {
            return false;
        }

        // set the size data in the Tilemap utility
        m_tilemap.init(m_width, m_height, m_tile_size);

        m_vertices.setPrimitiveType(sf::Quads);
        // resize the vertex array
        m_vertices.resize(m_width * m_height * 4);

        // Populate the array of index.
        history_append(game);

        assert(next_draw == 0);

        last_draw = next_draw;
        set_data(next_draw++);

        return true;
    }


    /**
     * Translate the game's cell data into an array of
     * ints which is appended to the history queue.
     */
    void history_append(const Game& game) {
        std::vector<int>& nex = m_history.emplace_back();
        nex.reserve(m_height * m_width);

        for (size_t j = 0; j < m_height; ++j) {
            for (size_t i = 0; i < m_width; ++i) {
                const Cell& cell = game.cell(i + j * m_width);
                nex.push_back(tr(cell));
            }
        }
    }


    /**
     * Send the array of tile index to the Tilemap utility which will
     * populate the VertexArray from it.
     *
     * @param history_index  the index inside of m_history of the grid
     * to be drawn.
     */
    void set_data(size_t history_index) {
        m_tilemap.set_tiles(m_history[history_index].data(), m_tileset, m_vertices);
    }

    /**
     * Use this to perform a data update query (simply calls history_append
     * and set_data).
     */
    void append_data(const Game& game) {
        assert(m_vertices.getPrimitiveType() == sf::Quads);

        history_append(game);

        assert(next_draw < m_history.size());
        assert(m_vertices.getVertexCount() == 4 * m_history[next_draw].size());

        assert(next_draw == last_draw + 1);

        last_draw = next_draw;
        set_data(next_draw++);
    }

    // std::function<void(void*)> callback_draw;
    // void* callback_draw_data;


private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // apply the transform
        states.transform *= getTransform();

        // apply the tileset texture
        states.texture = &m_tileset;

        // draw the vertex array
        target.draw(m_vertices, states);
    }

    unsigned int m_width;
    unsigned int m_height;
    sf::Vector2u m_tile_size;

    sf::VertexArray m_vertices;
    sf::Texture m_tileset;
    TileMap m_tilemap;

    std::deque<std::vector<int>> m_history;
    int next_draw = 0;
    int last_draw = -1;
};


#endif // VIEWER_H_
