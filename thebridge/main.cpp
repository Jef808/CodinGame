#include <algorithm>
#include <iostream>
#include <sstream>

#include "game.h"


using namespace CGbridge;




int main(int argc, char *argv[])
{
    enum class Mode { Local, Online };
    Mode mode;
    Game game;

    if (argc < 2)
    {
        std::cerr << "Online mode!" << std::endl;
        mode = Mode::Online;
        std::ios_base::sync_with_stdio(false);
        game.initial_input(std::cin);
    }
    else
    {
        std::cerr << "Local mode!" << std::endl;
        mode = Mode::Local;
        std::string fn = argv[1];
        std::ifstream ifs{ fn };
        if (!ifs) {
            std::cerr << "Failed to open input file "
                      << fn << std::endl;
            return EXIT_FAILURE;
        }
        game.initial_input(ifs);
    }

    return 0;
}
