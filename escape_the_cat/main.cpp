//#define DEBUG

#include "escape_cat.h"
#include "utilities.h"
#include "agent.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace escape_cat;

int norm2(const PointI_Euc& p) { return p.x*p.x + p.y*p.y; }

void pretty(const State& state, std::string buf) {
    auto mouse_pos = PointI_Euc{ rescale(state.mouse.euclidean_coordinates(), 0.1) };
    auto cat_pos = PointI_Euc{ rescale(state.cat.euclidean_coords(), 0.1) };

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

    Game game;
    game.init(cin);
    cin.ignore();

    Agent agent;

    Cat debug_cat = game.state().cat;
    Mouse debug_mouse = game.state().mouse;

    while (true) {
        game.update_state(cin);

#ifdef DEBUG
        if (not (debug_cat == game.state().cat))
            cerr << "Handling of action on Cat part of the state" << endl;
        if (not (debug_mouse == game.state().mouse))
            cerr << "Problem with the cat in our simulation" << endl;
#endif
        Point_Euc target = agent.choose_move(game, debug);
        agent.format_choice_for_submission(target, debug, output_buffer);

        cout << output_buffer << std::endl;
        //cerr << pretty(game.state()) << endl;

        //cerr << "Before game.step" << endl;
        // State next_state = game.step(target);
        // debug_cat = next_state.cat;
        // debug_mouse = next_state.mouse;
        // cerr << "New cat pos: " << new_cat.x << ' ' << new_cat.y
        //      << "\nNew mouse pos: " << new_mouse.x << ' ' << new_cat.y << endl;
    }

}
