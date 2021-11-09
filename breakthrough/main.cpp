#include "types.h"
#include "breakthrough.h"
#include "agent.h"
#include "eval.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
    constexpr bool debug_move_gen = true;

    std::string buf;

    Game game;
    game.init(std::cin);

    Agent agent(game);
    int s_depth = 0;
    int s_width = max_n_moves;

    StateInfo states[max_depth];
    StateInfo* st { &states[0] };
    bool done = false;

    while (!done) {

        Move move = agent.simple_best_move();
        //Move move = agent.best_move(s_depth, s_width, debug_best_move);

        std::cout << Game::view_move(move) << std::endl;

        game.apply(move, *st++);
        game.turn_init(std::cin, *st++);

        if (debug_move_gen && !game.test_move_gen()) {
            std::cerr << "Game::test_move_gen() failed:\n\n";
            game.test_move_gen(true);

            assert(false);
        }
        // game.show(std::cerr);

    }

    return EXIT_SUCCESS;
}
