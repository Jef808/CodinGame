/**
 * Simulate online play using `fakejudge.h'.
 */
#include "types.h"
#include "io.h"
#include "utils.h"
#include "fakejudge.h"

#include <iostream>
#include <sstream>
#include <vector>


using namespace sok2;
using namespace sok2::fakejudge;


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
        ss << argv[i];
    }

    Game game;
    ss >> game;

    Window bomb;
    ss >> bomb;

    FakeJudge judge{ game, bomb };

    return 0;
}
