#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>

#include "tb.h"
#include "agent.h"
#include "types.h"

#include "viewer.h"

using namespace tb;


State input_turn(std::istream& _in) {
    State state;
    //std::stringstream ss{};
    int x, y, a;  // x-coord, y-coord, active-or-not
    _in >> state.speed; _in.ignore();

    for (int i=0; i<params.start_bikes; ++i)
    {
        _in >> x >> y >> a; _in.ignore();

        //ss << x << ' ' << y << ' ' << a << std::endl;
        state.bikes[i] = (a == 1);
    }
    state.pos = x;
    return state;
}

void input_turn_ignore(std::istream& _in) {
    std::string buf;
    for (int i=0; i<params.start_bikes; ++i) {
        std::getline(_in, buf);
    }
}

/**
 * Helper class representing either std::cin or some std::ifstream.
 */
class Istream {
public:
    Istream(const std::string& fn = std::string()) :
        ifs{ fn.empty() ? std::nullopt : std::make_optional(std::ifstream{fn}) }
    {
        std::ios_base::sync_with_stdio(false);
        if (ifs) {
            std::cin.rdbuf(ifs->rdbuf());
        }
    }
    operator bool() {
        return std::cin.good() && (!ifs || ifs->is_open());
    }
    operator std::istream&() {
        return std::cin;
    }
private:
    std::optional<std::ifstream> ifs;
};

using namespace tb::viewer;

// std::ostream& operator<<(std::ostream& out, const Action a)
// {
//     using tb::Action;
//     switch (a) {
//     case Action::Wait:
//         return out << "Wait";
//     case Action::Slow:
//         return out << "Slow";
//     case Action::Speed:
//         return out << "Speed";
//     case Action::Jump:
//         return out << "Jump";
//     case Action::Up:
//         return out << "Up";
//     case Action::Down:
//         return out << "Down";
//     default:
//         return out << "None";
//     }
// }

std::ostream& operator<<(std::ostream& out, const ExtCell c) {
    switch(c) {
    case ExtCell::Bridge : out << '-'; break;
    case ExtCell::Hole   : out << '0'; break;
    case ExtCell::Bike_bridge : out << 'B'; break;
    case ExtCell::Bike_hole : out << 'X'; break;
    }
    return out;
}

int main(int argc, char *argv[])
{
    Istream is{ argc > 1 ? argv[1] : std::string() };

    if (argc > 1 && !is) {
        std::cerr << "Failed to open "
            << argv[1] << std::endl;
    }

    static constexpr int max_time_ms = 150;
    Game::init(is);
    State init_state = input_turn(is);
    Game game;
    Agent agent(game);

    // Could close the file here really
    game.set(init_state);

    agent.set_time_lim(max_time_ms);
    agent.init(20);     // Save 32mb for the transposition table

    auto action = agent.get_next();
    std::vector<Action> history;

    while (action != Action::None) {
        std::cout << action << std::endl;
        history.push_back(action);
        input_turn_ignore(is);
        action = agent.get_next();
    }

    //game.set(state_cpy);
    //game.view(std::cout, history);

    return 0;
}
