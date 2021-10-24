#ifndef DPVIEW_H_
#define DPVIEW_H_

#include <memory>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Texture.hpp>

enum class Resolution {
    Small,
    Medium,
    Big
};

namespace dp {
class Game;
struct GameParams;
struct DpData;
struct Entity;
struct Data;
}

namespace sf {
class RenderTarget;
class RenderStates;
}

class DpView : public sf::Drawable {
public:
    bool init(const dp::Game& game, Resolution resolution = Resolution::Small, int history_length = 50);

    void update(const dp::Data* data);

    virtual void draw(sf::RenderTarget& window, sf::RenderStates states) const;

private:
    const dp::GameParams* m_params;
    sf::VertexArray m_vertices;
    sf::Texture m_tileset;
    std::vector<int> m_background;
    std::vector<int> m_buffer;
    int tile_size;
    int width;
    int height;

    void set(const std::vector<int>& buffer);
};

#endif // DPVIEW_H_
