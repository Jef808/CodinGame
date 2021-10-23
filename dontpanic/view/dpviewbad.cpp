#include "dpview.h"
#include "dp.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <string_view>
#include <vector>
#include <utility>

constexpr inline auto texture_small = "resources/dp_tileset32.png";
constexpr inline auto texture_medium = "resources/dp_tileset64.png";
constexpr inline auto texture_big = "resources/dp_tileset128.png";


class DpView::Manager
{
public:
    void init(const dp::Game& game, sf::Vector2u tile_size, sf::Vector2u texture_size, int history_length, sf::VertexArray& vertices);
    void apply(dp::Action action, sf::VertexArray& vertices);
    void undo(sf::VertexArray& vertices);
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    const dp::GameParams* m_params;
    int m_width;
    int m_height;
    sf::Vector2u m_tile_size;
    sf::Vector2u m_texture_size;

    using Changes = std::vector<std::tuple<size_t, int, int>>;
    std::deque<Changes> m_stack;
    int m_history_length;

    void get_changes(dp::Action action);
    void update(Changes& changes, sf::VertexArray& vertices, int n = 1);
};

void DpView::deleteManager::operator() (DpView::Manager* mgr) const {
    delete mgr;
}

// 0: empty, 1: wall_left, 2: wall_right, 3: entry
// 4: exit, 5: elevator, 6: clone left, 7: clone right
void DpView::Manager::init(const dp::Game& game, sf::Vector2u tile_size, sf::Vector2u texture_size, int history_length, sf::VertexArray& vertices)
{
    m_params = game.get_params();
    m_history_length = history_length;
    m_height = m_params->height;
    m_width = m_params->width + 2;
    m_tile_size = tile_size;
    m_texture_size = texture_size;

    m_stack.push_back(Changes());
    Changes& changes = m_stack.back();

    changes.reserve(m_width * m_height);

    // initialize the first change to all empty
    for (int i=0; i < m_width * m_height; ++i)
        changes.push_back({ i, 0, 0 });

    // add the walls
    for (int i = 0; i < m_height; ++i) {
        get<2>(changes[i * m_width]) = 1;
        get<2>(changes[(i + 1) * m_width - 1]) = 2;
    }

    // add the elevators
    for (const auto& el : m_params->elevators) {
        get<2>(changes[(m_height - 1 - el.floor) * m_width + (el.pos + 1)]) = 5;
    }

    // add the entry and exit
    int in_p = m_params->entry_pos, out_p = m_params->exit_pos, out_f = m_params->exit_floor;
    get<2>(changes[(m_height - 1) * m_width + (in_p + 1)]) = 3;
    get<2>(changes[(m_height - 1 - out_f) * m_width + (out_p + 1)]) = 4;

    // set the drawing position of the quads
    for (int i = 0; i < m_height; ++i)
        for (int j = 0; j < m_width; ++j) {

            sf::Vertex* quad = &vertices[(i * m_width + j) * 4];

            // the position at which the quad is to be drawn,
            // indexing clockwise from the upper left vertex
            quad[0].position = sf::Vector2f(j * tile_size.x, i * tile_size.y);
            quad[1].position = sf::Vector2f((j + 1) * tile_size.x, i * tile_size.y);
            quad[2].position = sf::Vector2f((j + 1) * tile_size.x, (i + 1) * tile_size.y);
            quad[3].position = sf::Vector2f(j * tile_size.x, (i + 1) * tile_size.y);
        }

    // set the texture positions
    update(changes, vertices);
}

void DpView::Manager::apply(dp::Action action, sf::VertexArray& vertices)
{
    if (action == dp::Action::None)
        return;

    get_changes(action);
    update(m_stack.back(), vertices);
}

void DpView::Manager::update(Changes& changes, sf::VertexArray& vertices, int n)
{
    assert(!changes.empty());

    for (int i=0; i<changes.size(); ++i)
    {
        auto [ndx, cur, next] = changes[i];

        // the quad that needs to be updated
        sf::Vertex* quad = &vertices[ndx * 4];

        // save the current texture tile
        int col = quad[0].texCoords.x * (m_texture_size.x / m_tile_size.x);
        int row = quad[0].texCoords.y * (m_texture_size.y / m_tile_size.y);
        get<1>(changes[i]) = row * (m_texture_size.x / m_tile_size.x) + col;

        // the row/column coordinates of the new texture tile
        int tx = next % (m_texture_size.x / m_tile_size.x);
        int ty = next / (m_texture_size.x / m_tile_size.x);

        // make the quad point to the corresponding squares in the texture itself
        quad[0].texCoords = sf::Vector2f(tx * m_tile_size.x, ty * m_tile_size.y);
        quad[1].texCoords = sf::Vector2f((tx + 1) * m_tile_size.x, ty * m_tile_size.y);
        quad[2].texCoords = sf::Vector2f((tx + 1) * m_tile_size.x, (ty + 1) * m_tile_size.y);
        quad[3].texCoords = sf::Vector2f(tx * m_tile_size.x, (ty + 1) * m_tile_size.y);
    }
}

void DpView::Manager::undo(sf::VertexArray& vertices)
{
    const Changes& last_changes = m_stack.back();

    for (auto [ndx, last, cur] : last_changes)
    {
        // the quad that needs to be updated
        sf::Vertex* quad = &vertices[ndx * 4];

        // the row/column coordinates of the target texture tile
        int tx = last % (m_texture_size.x / m_tile_size.x);
        int ty = last / (m_texture_size.x / m_tile_size.x);

        // make the quad point to the corresponding squares in the texture itself
        quad[0].texCoords = sf::Vector2f(tx * m_tile_size.x, ty * m_tile_size.y);
        quad[1].texCoords = sf::Vector2f((tx + 1) * m_tile_size.x, ty * m_tile_size.y);
        quad[2].texCoords = sf::Vector2f((tx + 1) * m_tile_size.x, (ty + 1) * m_tile_size.y);
        quad[3].texCoords = sf::Vector2f(tx * m_tile_size.x, (ty + 1) * m_tile_size.y);
    }

    // pop the undo stack
    m_stack.pop_back();
}

void DpView::Manager::get_changes(dp::Action action)
{
    m_stack.push_back(Changes());
}

bool DpView::init(const dp::Game& game, Resolution resolution, int history_length)
{
    std::string_view fn;
    sf::Vector2u tile_size;
    switch (resolution) {
    case Resolution::Small:
        fn = texture_small;
        tile_size.x = tile_size.y = 32;
        break;
    case Resolution::Medium:
        fn = texture_medium;
        tile_size.x = tile_size.y = 64;
        break;
    case Resolution::Big:
        fn = texture_big;
        tile_size.x = tile_size.y = 128;
        break;
    default: {
        std::cerr << "unknown texture option" << std::endl;
        return false;
    }
    }

    if (!m_tileset.loadFromFile(fn.data()))
        return false;

    m_vertices.setPrimitiveType(sf::Quads);
    m_vertices.resize((game.get_params()->width+2) * game.get_params()->height * 4);

    mgr->init(game, tile_size, m_tileset.getSize(), history_length, m_vertices);

    return true;
}
