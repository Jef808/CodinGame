#include "tb.h"
#include "grid/viewer.h"

#include <iostream>
#include <fstream>
#include <string>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>


constexpr auto tileset32 = "resources/tileset_tb32.png";
constexpr auto tileset64 = "resources/tileset_tb64.png";

enum class Tile {
    Bridge, Hole, Bike, BikeHole
};

using namespace tb;

int main(int argc, char* argv[])
{
    const int window_width = 1920, window_height = 1080 / 2;

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
    if (!viewer.init("resources/tileset_tb32.png", width, height, Grid::TileSize::Small, Grid::YDirection::Down)) {
        return EXIT_FAILURE;
    }

    for (auto [tile, index] : { std::pair{ Tile::Bridge,   0 },
                                std::pair{ Tile::Hole,     1 },
                                std::pair{ Tile::Bike,     2 },
                                std::pair{ Tile::BikeHole, 3 } }) {
        viewer.register_tile(tile, index);
    }

    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {
            Tile tile = road[y][x] == Cell::Bridge ? Tile::Bridge : Tile::Hole;
            viewer.set_tilepos_bg(tile, x, y);
        }
    }

    std::array<State, Max_depth> states;
    states[0] = *game.state();
    State* st = &states[0];

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "The Bridge - road");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Escape:
                    case sf::Keyboard::Q:
                        window.close();
                        break;
                    case sf::Keyboard::Space:
                        game.apply(*st, Action::Jump, *(st + 1));
                        ++st;
                        break;
                    case sf::Keyboard::K:
                        game.apply(*st, Action::Up, *(st + 1));
                        ++st;
                        break;
                    case sf::Keyboard::J:
                        game.apply(*st, Action::Down, *(st + 1));
                        ++st;
                        break;
                    case sf::Keyboard::L:
                        game.apply(*st, Action::Speed, *(st + 1));
                        ++st;
                        break;
                    case sf::Keyboard::H:
                        game.apply(*st, Action::Slow, *(st + 1));
                        ++st;
                        break;
                    case sf::Keyboard::BackSpace:
                        --st;
                        break;
                    case sf::Keyboard::R:
                        st = &states[0];
                    default:
                        break;
                }
            }
        }

        viewer.reset_fg();

        for (int y=0; y<4; ++y) {
            if (st->bikes[y]) {
                Tile tile = road[y][st->pos] == Cell::Bridge
                    ? Tile::Bike
                    : Tile::BikeHole;
                viewer.set_tilepos_fg(tile, st->pos, y);
            }
        }

        viewer.freeze();

        viewer.set_message("The Bridge: ");

        window.clear();
        window.draw(viewer);
        window.display();
    }

    return EXIT_SUCCESS;
}
