#include "dpview.h"
#include "dp.h"

#include <string_view>

constexpr auto small = "resources/dp_tileset32.png";
constexpr auto medium = "resources/dp_tileset64.png";
constexpr auto big = "resources/dp_tileset128.png";

bool DpView::init(const dp::Game& game, Resolution resolution, int history_length)
{
    m_params = game.get_params();

    std::string_view fn;
    switch(resolution) {
        case Resolution::Small: tile_size = 32; fn = small; break;
        case Resolution::Medium: tile_size = 64; fn = medium; break;
        case Resolution::Big: tile_size = 128; fn = big; break;
    }

    if (!m_tileset.loadFromFile(fn.data()))
        return false;

    width = m_params->width + 2;
    height = m_params->height;

    m_vertices.setPrimitiveType(sf::Quads);
    m_vertices.resize(width * height * 4);

    // set the drawing position of the quads
    for (int i = 0; i < height; ++i)
        for (int j = 0; j < width; ++j) {

            sf::Vertex* quad = &m_vertices[(i * width + j) * 4];

            // the position at which the quad is to be drawn,
            // indexing clockwise from the upper left vertex
            quad[0].position = sf::Vector2f(j * tile_size, i * tile_size);
            quad[1].position = sf::Vector2f((j + 1) * tile_size, i * tile_size);
            quad[2].position = sf::Vector2f((j + 1) * tile_size, (i + 1) * tile_size);
            quad[3].position = sf::Vector2f(j * tile_size, (i + 1) * tile_size);

        }

    update(*game.state());

    return true;
}

void DpView::set_tile(int pos, int floor, int tile)
{
    int i = height - 1 - floor;
    int j = pos + 1;

    m_buffer[i * width + j] = tile;

    sf::Vertex* quad = &m_vertices[(i * width + j) * 4];

    // the row/column coordinates of the target texture tile
    int tx = tile % (m_tileset.getSize().x / tile_size);
    int ty = tile / (m_tileset.getSize().x / tile_size);

    // make the quad point to the corresponding squares in the texture itself
    quad[0].texCoords = sf::Vector2f(tx * tile_size, ty * tile_size);
    quad[1].texCoords = sf::Vector2f((tx + 1) * tile_size, ty * tile_size);
    quad[2].texCoords = sf::Vector2f((tx + 1) * tile_size, (ty + 1) * tile_size);
    quad[3].texCoords = sf::Vector2f(tx * tile_size, (ty + 1) * tile_size);
}

/// TODO: This can be done only with a GameParams object.
void DpView::update(const dp::State& state) {
    m_buffer.clear();
    m_buffer.reserve(width * height);
    std::fill_n(std::back_inserter(m_buffer), width * height, 0);

    // add the walls
    for (int i = 0; i < height; ++i) {
        m_buffer[i * width] = 1;
        m_buffer[(i+1) * width - 1] = 2;
    }

    // add the elevators
    for (const auto& el : m_params->elevators) {
        m_buffer[(height - 1 - el.floor) * width + (el.pos + 1)] = 5;
    }

    // add the entry and exit
    int in_p = m_params->entry_pos, out_p = m_params->exit_pos, out_f = m_params->exit_floor;
    m_buffer[(height - 1) * width + (in_p + 1)] = 3;
    m_buffer[(height - 1 - out_f) * width + (out_p + 1)] = 4;

    for (int i = 0; i < height; ++i)
        for (int j = 0; j < width; ++j) {

            sf::Vertex* quad = &m_vertices[(i * width + j) * 4];

            // the row/column coordinates of the target texture tile
            int tx = m_buffer[i * width + j] % (m_tileset.getSize().x / tile_size);
            int ty = m_buffer[i * width + j] / (m_tileset.getSize().x / tile_size);

            // make the quad point to the corresponding squares in the texture itself
            quad[0].texCoords = sf::Vector2f(tx * tile_size, ty * tile_size);
            quad[1].texCoords = sf::Vector2f((tx + 1) * tile_size, ty * tile_size);
            quad[2].texCoords = sf::Vector2f((tx + 1) * tile_size, (ty + 1) * tile_size);
            quad[3].texCoords = sf::Vector2f(tx * tile_size, (ty + 1) * tile_size);
        }
}

void DpView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.texture = &m_tileset;
    target.draw(m_vertices, states);
}
