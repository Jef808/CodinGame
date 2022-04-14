#include "escape_cat.h"
#include "utilities.h"
#include "agent.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace escape_cat;


std::string pretty(const State& state) {
    Point mouse = state.mouse.point_position();
    Point cat = state.cat.get_point_position();

    rescale(mouse, 0.1);
    rescale(cat, 0.1);

    PointI mouse_ = mouse;
    PointI cat_ = cat;

    mouse_.x += 50;
    cat_.x += 50;

    std::stringstream ss;

    for (int x = 0; x < 100; ++x) {
        for (int y = 0; y < 55; ++y) {
            if (mouse_.x == x && mouse_.y == y) {
                ss << "M";
            }
            else if (cat.x == x && cat.y == y) {
                    ss << "C";
            }
            else {
                int norm2 = x*x + y*y;
                if (norm2 < 2500) {
                        ss << ".";
                }
                else if (norm2 < 2504){
                        ss << "o";
                }
                else
                    ss << ".";
            }
        }
        ss << '\n';
    }

    return ss.str();
}

/**
 * The way we approach the problem, each action consists of specifying a target point
 * on the circle boundary.
 *
 * We write down what quantity should be minimized/maximized, what constraints should
 * be statisfied, and move on from there.
 */



int main() {
    using std::cin;
    using std::cout;
    using std::cerr;
    using std::endl;

    Game game;
    game.init(cin);
    cin.ignore();

    Agent agent;

    // For debuging
    std::string debug;
    Cat* debug_cat = nullptr;
    Mouse* debug_mouse = nullptr;

    while (true) {
        game.update_state(cin);

        {
            if (*debug_cat != game.state().cat)
               cerr << "Handling of action on Cat part of theh state" << endl;
            if (*debug_mouse != game.state().mouse)
                cerr << "Problem with the cat in our simulation" << endl;
        }


        Point target = agent.choose_move(game, debug);
        agent.output_choice(cout, target, game.state());

        cerr << "Agent chose target " << target.x << ' ' << target.y << ' ' << debug << endl;

//        cerr << pretty(game.state()) << endl;

        cerr << "Collected the drawing data" << endl;

        cerr << "Before game.step" << endl;
        State next_state = game.step(target);
        debug_cat = &next_state.cat;
        debug_mouse = &next_state.mouse;
        // cerr << "New cat pos: " << new_cat.x << ' ' << new_cat.y
        //      << "\nNew mouse pos: " << new_mouse.x << ' ' << new_cat.y << endl;
    }

}
