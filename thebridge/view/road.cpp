#include "tb.h"
#include "grid/viewer.h"

#include <iostream>
#include <fstream>
#include <string>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>


enum class Tile {
    Bridge, Hole
};

using namespace tb;

int main(int argc, char* argv[])
{
    const int window_width = 800, window_height = 600;

    if (argc < 2) {
        std::cerr << "Main: Input file needed!" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream ifs { argv[1] };
    if (!ifs) {
        std::cerr << "Main: Failed to open input file!" << std::endl;
        return EXIT_FAILURE;
    }

    Game game;
    game.init(ifs);
    const auto& road = game.get_road();

    int width = road[0].size();
    int height = 4;

    Grid::Viewer<Tile> viewer;
    if (!viewer.init("resources/tileset_tb.png", width, height, Grid::TileSize::Medium)) {
        return EXIT_FAILURE;
    }

    for (auto [tile, index] : { std::pair{ Tile::Bridge, 0 }, std::pair{ Tile::Hole, 1 } }) {
        viewer.register_tile(tile, index);
    }

    std::cout << "road object:\n";

    for (const auto& row : road) {
        for (auto t : row) {
            std::cout << (t == Cell::Bridge ? '-' : 'O');
        }
        std::cout << std::endl;
    }

    std::cout << "\nSetting viewer:" << std::endl;

    viewer.show_tilemap(std::cout);

    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {
            Tile tile = road[y][x] == Cell::Bridge ? Tile::Bridge : Tile::Hole;
            viewer.set_tilepos_bg(tile, x, y);
        }
    }

    viewer.show_status(std::cout);

    viewer.reset();

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "The Bridge - road");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
                if (event.key.code == sf::Keyboard::Escape
                    || event.key.code == sf::Keyboard::Q)
                    window.close();
        }

        viewer.set_message("The Bridge");

        window.clear();
        window.draw(viewer);
        window.display();
    }

    return EXIT_SUCCESS;
}
