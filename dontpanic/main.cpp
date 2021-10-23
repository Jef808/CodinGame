#define RUNNING_OFFLINE 1
#define EXTRACTING_ONLINE_DATA 0

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "agent.h"
#include "dp.h"

#if RUNNING_OFFLINE
#include "view/dpview.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <fmt/format.h>

void show(const dp::Game& game, DpView& viewer)
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Don't Panic!");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(viewer);
        window.display();

        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // const dp::State* s = game.state();

        // viewer.set_tile(s->pos + 1, s->floor, 7);

        // window.clear();
        // window.draw(viewer);
        // window.display();

        // std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // viewer.set_tile(s->pos + 1, s->floor, 0);
        // viewer.set_tile(s->pos + 2, s->floor, 7);

        // window.clear();
        // window.draw(viewer);
        // window.display();

        // std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // viewer.set_tile(s->pos + 2, s->floor, 0);

        // window.clear();
        // window.draw(viewer);
        // window.display();

        // std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // viewer.set_tile(s->pos + 3, s->floor+1, 7);

        // window.clear();
        // window.draw(viewer);
        // window.display();

        // std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // viewer.set_tile(s->pos + 3, s->floor+1, 0);
        // viewer.set_tile(s->pos + 4, s->floor+1, 7);
        // viewer.set_tile(s->pos + 1, s->floor, 7);

        // window.clear();
        // window.draw(viewer);
        // window.display();

        // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

        // return;
    }
}

#endif


std::ostream& operator<<(std::ostream&, const dp::Action);

void ignore_turn(std::istream& _in)
{
    static std::string buf;
    buf.clear();
    std::getline(_in, buf);
    _in.ignore();
}

void solve()
{
    Agent agent;

#if RUNNING_OFFLINE

    for (int i = 0; i < 100; ++i) {
        auto action = agent.best_choice();
    }

    return;

#endif

    for (int i = 0; i < 100; ++i) {
        auto action = agent.best_choice();
        std::cout << action << std::endl;

        ignore_turn(std::cin);
    }
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
        fmt::print("Failed to open input file {}", fn);
        return EXIT_FAILURE;
    }

    Game game;
    game.init(ifs);

    DpView viewer;
    if (viewer.init(game, Resolution::Medium))
    {
        fmt::print("Successfully initialized the viewer");
        show(game, viewer);
    }
    else {
        fmt::print("Failed to initialise the viewer");
    }

/// Running Online
#else
#if EXTRACTING_ONLINE_DATA

    extract_online_init();
    return EXIT_SUCCESS;

#else

    Game game;
    game.init(std::cin);

#endif
#endif

    /// Main loop
    Agent::init(game);
    solve();

    std::cerr << "Exiting program"
              << std::endl;

    return EXIT_SUCCESS;
}
