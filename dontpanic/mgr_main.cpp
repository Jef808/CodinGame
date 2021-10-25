#include "view/dpview.h"
#include "dp.h"
#include "mgr.h"

#include <chrono>
#include <cassert>
#include <iostream>
#include <fstream>
#include <thread>
#include <random>


#include <fmt/format.h>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

using namespace dp;

void main_loop(DpMgr& mgr)
{
    while (true)
    {
        if (mgr.pre_input())
        {
            // output state here

            Action a = Action::Wait;

            mgr.input(a, std::cerr);

            mgr.post_input();
        }

        switch(mgr.status) {
            case DpMgr::status::Won:
                std::cout << "Success!"
                          << std::endl;
                return;
            case DpMgr::status::Lost:
                std::cout << "Game over"
                          << std::endl;
                return;
            case DpMgr::status::Error:
                std::cout << "Error"
                          << std::endl;
                return;
            default:
                continue;
        }
    }
}


int main(int argc, char* argv[])
{
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

    DpMgr mgr;
    mgr.load(game);
    //assert(mgr.status == DpMgr::status::Initialized);

    DpView viewer;
    if (!viewer.init(game, Resolution::Small))
    {
        fmt::print("Failed to initialise the viewer");
    }

    sf::RenderWindow window(sf::VideoMode(800, 600), "Don't Panic!");

    Action action;
    int action_i;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(viewer);
        window.display();

        // Do some work to get actions here
        if (mgr.pre_input())
        {
            action_i = -1;
            while (action_i < 0 || action_i > 4) {
                std::cout << "1: Wait, 2: Block, 3: Elevator, 0: quit"
                          << std::endl;
                std::cin >> action_i;
                std::cin.ignore();
            }
            if (action_i == 0) {
                window.close();
                return EXIT_SUCCESS;
            }
            // action_i = std::rand() % 3;
            // action = Action(action_i);

            if (!mgr.input(Action(action_i), std::cerr))
            {
                std::cout << "Invalid action, one more change...\n" << std::endl;

                action_i = -1;
                while (action_i < 0 || action_i > 4) {
                    std::cout << "1: Wait, 2: Block, 3: Elevator, 4: quit"
                              << std::endl;
                    std::cin >> action_i;
                    std::cin.ignore();
                }

                if (action_i == 4 || !mgr.input(Action(action_i), std::cerr)) {
                    std::cout << "Exiting program"
                        << std::endl;
                    window.close();
                    return EXIT_SUCCESS;
                }
            }

            // Process the new state
            mgr.post_input();

            viewer.update(mgr.dump());

            if (mgr.status == DpMgr::status::Won) {
                std::cout << "Game Won!!!"
                    << std::endl;

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                window.close();
                return EXIT_SUCCESS;
            }
        }
        else
        {
            std::cerr << "Game over"
                << std::endl;

            window.close();

            return EXIT_SUCCESS;
        }
    }

    return 0;
}
