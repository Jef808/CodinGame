#define RUNNING_OFFLINE 1
#define EXTRACTING_ONLINE_DATA 0

#include "tb.h"
#include "agent.h"

#include <string>
#include <iostream>
#include <fstream>

void ignore_turn(const tb::Game& game, std::istream& _in)
{
    std::string buf;
    for (int i = 0; i < game.parameters()->start_bikes + 1; ++i)
        std::getline(_in, buf);
}


using namespace tb;

int main(int argc, char* argv[])
{
#if RUNNING_OFFLINE

    const int turn_time_ms = 0;

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

#else

    const int turn_time_ms = 150;

    Game game;
    game.init(std::cin);

#endif

    Agent agent;
    agent.solve(game, turn_time_ms);

    return EXIT_SUCCESS;
}
