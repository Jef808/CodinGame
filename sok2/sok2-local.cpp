/**
 * Simulate online play using `referee.h'.
 */
#include "types.h"
#include "io.h"
#include "referee.h"
#include "viewer.h"

#include <iostream>
#include <sstream>


using namespace sok2;
using namespace sok2::viewer;


int main(int argc, char *argv[]) {
    using std::cin;
    using std::cout;
    using std::endl;

    if (argc < 7) {
        cout << "USAGE: "
             << argv[0]
             << " WIDTH HEIGHT N_JUMPS INIT_X INIT_Y BOMB_X BOMB_Y"
             << endl;
        return EXIT_FAILURE;
    }

    std::stringstream ss;
    for (int i=1; i<8; ++i) {
        ss << argv[i] << ' ';
    }

    Game game;
    ss >> game;

    Window bomb;
    ss >> bomb;

    cout << "Initial position: " << game.current_pos << '\n'
         << "Bomb position: " << bomb << endl;

    Referee referee{ game, bomb };
    Viewer viewer{ referee };

    viewer.view_legend(cout);
    viewer.view(cout);

    for (;;) {
        cout << "Pick a move" << endl;

        referee.process_turn(cin, cout);

        viewer.view_legend(cout);
        viewer.view(cout);
    }

    return 0;
}
