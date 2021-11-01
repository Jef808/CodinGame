#define RUNNING_OFFLINE 1

#include "tb.h"
#include "agent.h"

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>


std::string buf;

void ignore_turn(const tb::Game& game, std::istream& _in)
{
    for (int i = 0; i < game.parameters()->start_bikes + 1; ++i)
        std::getline(_in, buf);
}

std::ostream& operator<<(std::ostream& _out, tb::Action a)
{
    using tb::Action;
        switch (a) {
    // case Action::Wait:
    //     wait(s);
    //     break;
    case Action::Slow:
        return _out << "SLOW";
    case Action::Speed:
        return _out << "SPEED";
    case Action::Jump:
        return _out << "JUMP";
    case Action::Up:
        return _out << "UP";
    case Action::Down:
        return _out << "DOWN";
    default:
        assert(false);
    }
}


using namespace tb;

int main(int argc, char* argv[])
{
#if RUNNING_OFFLINE

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

    State state[2];
    state[0] = state[1] = *game.state();
    int count = 0;

#else

    Game game;
    game.init(std::cin);

#endif

    Agent agent;
    agent.init(game);
    agent.search(game);

    bool done = false;

    while (!done) {

        Action action = agent.best_action();

#if RUNNING_OFFLINE

        game.apply(state[count % 2], action, state[(count + 1) % 2]);
        ++count;

        bool lost = std::count(state[count % 2].bikes.begin(), state[count % 2].bikes.end(), 1) < game.parameters()->min_bikes;
        bool won = state[count % 2].pos >= game.get_road()[0].size() - 1;

        if (lost) {
            std::cerr << "Lost" << std::endl;
            done = true;
        }
        else if (won) {
            std::cerr << "Won"
                      << std::endl;
            done = true;
        }

        std::cout << action << std::endl;
        game.show(std::cout, state[count % 2]);

#else
        std::cout << action << std::endl;
        ignore_turn(game, std::cin);

#endif

    }

    return EXIT_SUCCESS;
}
