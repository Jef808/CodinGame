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
#include <fmt/format.h>
#endif

std::ostream& operator<<(std::ostream& _out, const dp::Action a)
{
    switch (a) {
        case dp::Action::Wait:
            return _out << "WAIT";
        case dp::Action::Block:
            return _out << "BLOCK";
        case dp::Action::Elevator:
            return _out << "ELEVATOR";
        default:
            return throw "Action::None", _out << "WARNING: Chose Action::None";
    }
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
    Agent agent;
    agent.init(game);

    agent.search();

    bool done = false;
    while (!done) {
        Action action = agent.best_choice();
        done = action == Action::None;
        if (done)
            break;
        std::cout << action
                  << std::endl;
    }

    std::cerr << "Exiting program"
              << std::endl;

    return EXIT_SUCCESS;
}
