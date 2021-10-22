#define RUNNING_OFFLINE 1
#define EXTRACTING_ONLINE_DATA 0

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include "types.h"
#include "dp.h"
#include "agent.h"

#if RUNNING_OFFLINE
  #include <fmt/core.h>
  #include <fmt/format.h>
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

    for (int i=0; i<100; ++i)
    {
        auto action = agent.best_choice();
    }

    return;

#endif

    for (int i=0; i<100; ++i)
    {
        auto action = agent.best_choice();
        std::cout << action << std::endl;

        ignore_turn(std::cin);
    }
}



using namespace dp;

int main(int argc, char *argv[]) {

/// Running Offline
#if RUNNING_OFFLINE
    if (argc < 2)
    {
        fmt::print(stderr, "USAGE: {} [Test number]\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string fn;
    fmt::format_to(std::back_inserter(fn), "../data/test{}.txt", argv[1]);

    std::ifstream ifs{ fn.data() };

    if (!ifs) {
        fmt::print("Failed to open input file {}", fn);
        return EXIT_FAILURE;
    }

    Game game;
    game.init(ifs);

    // game.view();
    // fmt::print("{}\n", "Successfully initialized the view");

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

    return EXIT_SUCCESS;
}
