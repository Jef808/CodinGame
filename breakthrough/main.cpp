#include "types.h"
#include "breakthrough.h"
#include "agent.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

struct AgentRandom {
    AgentRandom(Game& _game)
        : game(_game)
    {
    }

    Move best_move(int depth, int width) {
        auto [beg, end] = game.valid_moves();
        int n_moves = std::distance(beg, end);
        return *(beg + rand() % n_moves);
    }

private:
    Game& game;
};

int main(int argc, char* argv[])
{
    constexpr bool debug_best_move = false;
    constexpr bool debug_move_gen =  false;

    std::string buf;

    Game game;
    game.init(std::cin);

    //AgentRandom agent(game);
    Agent agent(game);
    int s_depth = 6;
    int s_width = 12;

    StateInfo states[max_depth];
    StateInfo* st { &states[0] };
    bool done = false;

    while (!done) {

        //Move move = agent.best_move(s_depth, s_width);
        Move move = agent.simple_best_move(debug_best_move);

        std::cout << game.make_move(move) << std::endl;

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
