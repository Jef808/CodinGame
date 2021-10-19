#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include "types.h"
#include "dp.h"
#include "agent.h"

#if FMT_ENABLED
  #include "viewutils.h"
#endif


namespace {

void ignore_turn(std::istream& _in)
{
    static std::string buf;
    buf.clear();
    std::getline(_in, buf);
}

void solve()
{
    dp::Agent agent;
    for (int i=0; i<100; ++i)
    {
        std::cout << agent.best_choice() << std::endl;

    #if RUNNING_OFFLINE
        return;
    #endif

        ignore_turn(std::cin);
    }
}


}  // namespace


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

    std::ifstream ifs{ fn };

    if (!ifs) {
        ifs.clear();
        fn = fn.substr(3);
        ifs.open(fn, std::ios_base::in);
        if (!ifs) {
            fmt::print("Failed to open both {} and ../{}\n", fn, fn);
            return EXIT_FAILURE;
        }
    }

    //fmt::print("Successfully opened the file");

    Game game;
    game.init(ifs);

    game.view();
    fmt::print("{}\n", "Successfully initialized the view");

    fmt::format("{}\n", "Starting main loop");

/// Running Online
#else
  #if EXTRACTING_ONLINE_DATA
    extract_online_init(std::ostream&);
    return EXIT_SUCCESS;
  #else
    Game game;
    game.init(std::cin);

  #endif
#endif

/// Main loop
    Agent::init(game);
    solve();

    std::cerr << "Exiting the program."
        << std::endl;

    return EXIT_SUCCESS;
}
