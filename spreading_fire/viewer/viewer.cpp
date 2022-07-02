#include "tilemap.h"

int main()
{
    // create the window
    sf::RenderWindow window(sf::VideoMode(512, 256), "Tilemap");

    // define the level with an array of tile indices
    const int level[] =
    {
        0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 0,
        0, 1, 3, 1, 1, 0,
        0, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 2, 0,
        0, 0, 4, 2, 2, 0,
        0, 0, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0
    };

    // create the tilemap from the level definition
    TileMap map;
    if (!map.load(VIEWER_DATA_DIR "/tiles.png", sf::Vector2u(60, 60), level, 6, 8))
        return -1;

    // run the main loop
    while (window.isOpen())
    {
        // handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        // draw the map
        window.clear();
        window.draw(map);
        window.display();
    }

    return 0;
}
