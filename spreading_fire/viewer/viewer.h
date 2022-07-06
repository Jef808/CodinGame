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

    /**
     * Set up the size of the grids that will be drawn, and initialize
     * the initial view from the passed `game` parameter.
     */
    bool init(const Game& game) {
        // Clear out any previous data
        m_history.clear();

        if (!m_tileset.loadFromFile(VIEWER_DATA_DIR "/Tileset_384x64.png")) {
            return false;
        }

        if (!m_font.loadFromFile(VIEWER_DATA_DIR "/JetBrainsMono-Bold.ttf")) {
            return false;
        }

        m_width = game.width();
        m_height = game.height();
        m_tile_size = sf::Vector2u(64, 64);
        m_text_size = 30;
        m_window_width = m_tile_size.x * m_width;
        m_window_height = m_tile_size.y * m_height + 6 * m_text_size;
        m_message_pos_y = m_window_height + m_text_size;

        // set the size data in the Tilemap utility
        m_tilemap.init(m_width, m_height, m_tile_size);

        // resize the vertex array and set its primitive type to quads
        m_vertices.setPrimitiveType(sf::Quads);
        m_vertices.resize(m_width * m_height * 4);

        // Populate the array of index with the initial game state.
        history_append(game);

        draw_index = 0;
        populate_vertex_array();
        draw_index = 1;

        return true;
    }

    /**
     * Use this to perform a data update query (simply calls history_append
     * and set_data).
     */
    void set_data(const Game& game) {
        history_append(game);
        draw_index = m_history.size() - 1;
        populate_vertex_array();
    }

    /**
     * Increment the pointer to the grid that has to be drawn in `m_history`.
     */
    void next() {
        if (draw_index < m_history.size() - 1) {
            ++draw_index;
            populate_vertex_array();
        }
    }

    /**
     * Decrement the pointer to the grid that has to be drawn in `m_history`.
     */
    void previous() {
        if (draw_index > 0) {
            --draw_index;
            populate_vertex_array();
        }
    }

    const sf::Font& font() const { return m_font; }

    unsigned int window_width() const { return m_window_width; }
    unsigned int window_height() const { return m_window_height; }
    unsigned int text_size() const { return m_text_size; }
    sf::Vector2f get_text_pos(const std::string& text) {
        return {(float)m_message_pos_y, (float)text.size() * m_text_size};
    }

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
    unsigned int m_text_size;
    unsigned int m_message_pos_y;
    unsigned int m_window_width;
    unsigned int m_window_height;

    sf::Font m_font;
    sf::Texture m_tileset;

    sf::VertexArray m_vertices;
    TileMap m_tilemap;

    std::deque<std::vector<int>> m_history;
    int draw_index = 0;

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
     * Send the indicated array of tile index to the Tilemap utility which will
     * populate `m_vertices` from it.
     *
     * @param history_index  the index in `m_history` of the grid
     * to be drawn.
     */
    void populate_vertex_array() {
        m_tilemap.set_tiles(m_history[draw_index].data(), m_tileset, m_vertices);
    }
};


#endif // VIEWER_H_
