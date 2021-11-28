#include "types.h"
#include "breakthrough.h"
#include "eval.h"
#include "../utils/rand.h"

#include <fstream>
#include <iostream>

Rand::Util<int> rand_util;

Move random_move(const Game& game) {
    game.compute_valid_moves();
    auto [beg, end] = game.valid_moves();
    return *(beg + rand_util.get(0, std::distance(beg, end) - 1));
}

int main()
{
    Game game;
    std::ifstream ifs{ "data/test.txt" };
    if (!ifs) {
        std::cerr << "failed to open input file"
            << std::endl;
        return EXIT_FAILURE;
    }
    game.init(ifs);
    StateInfo states[max_depth];
    StateInfo* st{ &states[0] };

    while (true) {
        {
            auto [score, trace] = Eval::trace(game);
            std::cout << trace << std::endl;
        }
        Move white_move = random_move(game);
        game.apply(white_move, *st++);
        if (game.has_won<Player::White>()) {
            std::cerr << game.view()
                      << "\n\nWhite wins!" << std::endl;
            break;
        }
        {
            auto [score, trace] = Eval::trace(game);
            std::cout << trace << std::endl;
        }
        Move black_move = random_move(game);
        game.apply(black_move, *st++);
        if (game.has_won<Player::Black>()) {
            std::cerr << game.view()
                      << "\n\nBlack wins!" << std::endl;
            break;
        }
    }
}
