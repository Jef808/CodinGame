#include <cassert>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

//#if RUNNING_OFFLINE
 #include "view/dpview.h"
 #include <fmt/format.h>
 #include <SFML/Window/Event.hpp>
 #include <SFML/Graphics/RenderWindow.hpp>
 #include <SFML/Window/Keyboard.hpp>
//#endif

#include "dp.h"

using namespace dp;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fmt::print("USAGE: {} [Test number]\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string fn;
    fmt::format_to(std::back_inserter(fn), "../data/test{}.txt", argv[1]);

    std::ifstream ifs { fn.data() };

    if (!ifs) {
        fmt::print("Failed to open input file {}\n", fn);
        return EXIT_FAILURE;
    }

    Game game;
    game.init(ifs);

    const int screen_width = 1920;

    auto tile_size = [](Resolution res) {
        int ret;
        switch(res) {
            case Resolution::Small:  ret = 32;  break;
            case Resolution::Medium: ret = 64;  break;
            case Resolution::Big:    ret = 128; break;
            default: throw "Unknown resolution";
        }
        return ret;
    };

    Resolution res = Resolution::Big;
    int window_width  = (game.get_params()->width + 2) * tile_size(res);
    if (window_width > screen_width) {
        res = Resolution::Medium;
        window_width = (game.get_params()->width + 2) * tile_size(res);
        if (window_width > screen_width) {
            res = Resolution::Small;
            window_width = (game.get_params()->width + 2) * tile_size(res);
        }
    }

    int window_height = game.get_params()->height * tile_size(res) + 32;

    dp::DpView viewer;
    if (!viewer.init(game, res))
    {
        fmt::print("Failed to initialise the viewer");
        return EXIT_FAILURE;
    }

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Don't Panic!");

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
                if (event.key.code == sf::Keyboard::Escape
                    || event.key.code == sf::Keyboard::Q)
                    window.close();
        }

        window.clear();
        window.draw(viewer);
        window.display();
    }

    fmt::print("Exiting program");

    return EXIT_SUCCESS;
}
