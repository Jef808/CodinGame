//#define DEBUG
#include "utilities.h"
#include "escape_cat.h"
#include "agent.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace escape_cat;

int norm2(const PointI_Euc& p) { return p.x*p.x + p.y*p.y; }

void pretty(const State& state, std::string buf) {
    auto mouse_pos = PointI_Euc{ rescale(state.mouse, 0.1) };
    auto cat_pos = PointI_Euc{ rescale(state.cat, 0.1) };

    // translate because origin coordinate systems is centered at
    // the center of the circle
    mouse_pos.x += 50; mouse_pos.y += 50;
    cat_pos.x += 50; cat_pos.y += 50;

    std::stringstream ss{ buf };

    PointI_Euc counter{ 0, 0 };

    for (; counter.x < 100; ++counter.x) {
        for (; counter.y < 100; ++counter.y) {
            if (mouse_pos == counter)
                ss << "M";
            else if (cat_pos == counter) {
                ss << "C";
            }
            else if (norm2(counter) < 50*50) {
                ss << ".";
            } else if (norm2(counter) < 52*52) {
                ss << "o";
            }
            else
                ss << " ";
        }
        ss << '\n';
    }
}


int main() {
    using std::cin;
    using std::cout;
    using std::cerr;
    using std::endl;

    // For debuging
    std::string output_buffer;
    std::string debug;

    Game game{ cin };
    cin.ignore();

    Agent agent;

    while (true) {
        game.update_state(cin);

        Point_Euc target = agent.choose_move(game, debug);
        agent.format_choice_for_submission(target, debug, output_buffer);

        cout << output_buffer << std::endl;
    }

}
