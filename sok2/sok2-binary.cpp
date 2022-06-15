/**
 * Play against the local referee using two binary searches.
 */
#include "types.h"
#include "io.h"
#include "referee.h"
#include "viewer.h"
#include "agent.h"

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

    Agent agent{ game };

    bool going = true;

    std::string ibuf;
    std::string obuf;

    while (going) {
        Window move = agent.choose_move();

        ibuf.clear();
        obuf.clear();
        std::stringstream iss{ ibuf };
        std::stringstream oss{ obuf };

        iss << move;

        going = referee.process_turn(iss, oss);

        Heat move_heat;
        oss >> move_heat;

        cout << "Chose move " << move << '\n'
             << "..... " << move_heat << endl;

        agent.update_data(move_heat);

        viewer.view_legend(cout);
        viewer.view(cout);

        char c;
        cout << "Input any character to continue..." << endl;
        cin >> c;
        cin.ignore();
    }

    return EXIT_SUCCESS;
}
