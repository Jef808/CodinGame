#define RUNNING_OFFLINE 1
#define EXTRACTING_ONLINE_DATA 0

#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

#if RUNNING_OFFLINE
#include "mgr.h"
#include "view/dpview.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <fmt/format.h>
#endif

#include "agent.h"
#include "dp.h"

std::string to_string(const dp::Action a)
{
    switch (a) {
    case dp::Action::Wait:
        return "WAIT";
    case dp::Action::Block:
        return "BLOCK";
    case dp::Action::Elevator:
        return "ELEVATOR";
    default:
        return throw "Action::None", "WARNING: Chose Action::None";
    }
}

std::ostream& operator<<(std::ostream& _out, const dp::Action a)
{
    return _out << to_string(a);
}

void ignore_turn(std::istream& _in)
{
    static std::string buf;
    buf.clear();
    std::getline(_in, buf);
    _in.ignore();
}

using namespace dp;

int main(int argc, char* argv[])
{
/// Running Offline
#if RUNNING_OFFLINE
    if (argc < 2) {
        fmt::print(stderr, "USAGE: {} [Test number]\n", argv[0]);
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

    agent::init(game);

    auto start = std::chrono::steady_clock::now();

    agent::search();

    auto time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
    std::cerr << std::setprecision(4)
              << "Time taken: "
              << time / 1000 << "ms" << std::endl;

    dp::DpMgr mgr;
    mgr.load(game);

    const int screen_width = 1920;

    auto tile_size = [](Resolution res) {
        int ret;
        switch (res) {
        case Resolution::Small:
            ret = 32;
            break;
        case Resolution::Medium:
            ret = 64;
            break;
        case Resolution::Big:
            ret = 128;
            break;
        default:
            throw "Unknown resolution";
        }
        return ret;
    };

    Resolution res = Resolution::Big;
    int window_width = (game.get_params()->width + 2) * tile_size(res);
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
    if (!viewer.init(game, res)) {
        fmt::print("Failed to initialise the viewer");
        return EXIT_FAILURE;
    }

    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Don't Panic!");

    int delay = 500;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
                if (event.key.code == sf::Keyboard::F)
                    delay = delay > 100 ? delay - 100 : 0;
                if (event.key.code == sf::Keyboard::S)
                    delay += 100;
            }
        }

        window.clear();
        window.draw(viewer);
        window.display();

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));

        if (!mgr.pre_input()) {
            switch (mgr.status) {
            case DpMgr::status::Won:
                std::cerr << "Success!"
                          << std::endl;
                break;
            case DpMgr::status::Lost:
                std::cerr << "Failure!"
                          << std::endl;
                break;
            case DpMgr::status::Error:
                std::cerr << "Error"
                          << std::endl;
                break;
            default:
                assert(false);
            }

            window.close();
            break;
        }

        Action action = dp::agent::best_choice();

        if (action == Action::None) {
            std::cout << "Done with agent's action queue"
                      << std::endl;
            window.close();
            break;
        }

        if (!mgr.input(action, std::cerr)) {
            std::cout << "Received invalid action"
                      << std::endl;
            window.close();
            break;
        }

        mgr.post_input();

        viewer.update(mgr.dump(), to_string(action));
    }

    std::cerr << "Exiting program"
              << std::endl;

/// Running Online
#else
#if EXTRACTING_ONLINE_DATA

    extract_online_init();
    return EXIT_SUCCESS;

#else

    Game game;
    game.init(std::cin);

    agent::init(game);

    auto start = std::chrono::steady_clock::now();

    agent::search();

    Action action = agent::best_choice();
    while (action != Action::None) {
        std::cout << action
                  << std::endl;
        ignore_turn(std::cin);
        action = agent::best_choice();
    }

#endif
#endif

    return EXIT_SUCCESS;
}
