#include "dpview.h"
#include "dp.h"
#include "mgr.h"

#include <cassert>
#include <string_view>

constexpr auto small = "resources/dp_tileset32.png";
constexpr auto medium = "resources/dp_tileset64.png";
constexpr auto big = "resources/dp_tileset128.png";
constexpr auto dejaVuSans = "resources/DejaVuSans.ttf";

namespace dp {

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

    if (!m_font.loadFromFile(dejaVuSans))
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

    m_background.clear();
    m_background.reserve(width * height);

    // fill the window with empty tiles (Empty = 0)
    std::fill_n(std::back_inserter(m_background), width * height, 0);

    // add the walls (WallL = 1, WallR = 2)
    for (int i = 0; i < height; ++i) {
        m_background[i * width] = 1;
        m_background[(i+1) * width - 1] = 2;
    }

    // add the elevators (Elevator = 5)
    for (const auto& el : m_params->elevators) {
        m_background[(height - 1 - el.floor) * width + (el.pos + 1)] = 5;
    }

    // add the entry and exit (Entry = 3, Exit = 4)
    int in_p = m_params->entry_pos, out_p = m_params->exit_pos, out_f = m_params->exit_floor;
    m_background[(height - 1) * width + (in_p + 1)] = 3;
    m_background[(height - 1 - out_f) * width + (out_p + 1)] = 4;

    set(m_background);

    m_text.setFont(m_font);
    m_text.setCharacterSize(24);
    m_text.setFillColor(sf::Color::Red);
    m_text.setStyle(sf::Text::Bold);
    m_text.setPosition(8.0, tile_size * height + 8.0);

    return true;
}

void DpView::set(const std::vector<int>& buffer)
{
    for (int i = 0; i < height; ++i)
        for (int j = 0; j < width; ++j) {

            sf::Vertex* quad = &m_vertices[(i * width + j) * 4];

            // the row/column coordinates of the target texture tile
            int tx = buffer[i * width + j] % (m_tileset.getSize().x / tile_size);
            int ty = buffer[i * width + j] / (m_tileset.getSize().x / tile_size);

            // make the quad point to the corresponding squares in the texture itself
            quad[0].texCoords = sf::Vector2f(tx * tile_size, ty * tile_size);
            quad[1].texCoords = sf::Vector2f((tx + 1) * tile_size, ty * tile_size);
            quad[2].texCoords = sf::Vector2f((tx + 1) * tile_size, (ty + 1) * tile_size);
            quad[3].texCoords = sf::Vector2f(tx * tile_size, (ty + 1) * tile_size);
        }
}

// elevator: 5, cloneL: 6, cloneR: 7
void DpView::update(const dp::Data* data, const std::string& msg)
{
    m_buffer.clear();
    std::copy(m_background.begin(), m_background.end(), std::back_inserter(m_buffer));

    auto it_clones = data->entities.begin();
    auto it_elevators = it_clones + data->n_clones;
    auto it_blocked_clones = it_elevators + data->n_player_elevators;
    auto it_end = it_blocked_clones + data->n_blocked_clones;

    auto ndx = [w=width, h=m_params->height](int pos, int floor){ return (pos + 1) + w * (h - 1 - floor); };
    auto tr = [](const auto& e){ switch(e.type){
            case Type::Elevator: return 5;
            case Type::Clone: return *e.dir == Dir::Left ? 6 : 7;
            default: assert(false);
        }};

    std::for_each(it_clones, it_end, [&](const auto& e){
        m_buffer[ndx(e.pos, e.floor)] = tr(e);
    });

    set(m_buffer);

    m_text.setString(std::string(msg));
}

void DpView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.texture = &m_tileset;
    target.draw(m_vertices, states);
    target.draw(m_text);
}

}  // namespace dp
