#include <cassert>
#include <iostream>
#include <string_view>
#include <vector>

#include "types.h"
#include "game.h"
#include "agent.h"

using namespace spring;

auto
operator<<(std::ostream& _out, const Point& p) -> std::ostream&
{
    return _out << p.x << ' ' << p.y;
}

auto
operator<<(std::ostream& _out, const Action& action) -> std::ostream&
{
    _out << (action.type == Action::Type::WAIT   ? "WAIT"
             : action.type == Action::Type::MOVE ? "MOVE"
                                                 : "SPELL");
    if (action.type == Action::Type::SPELL) {
        _out << ' '
             << (action.spell.type == Spell::Type::WIND     ? "WIND"
                 : action.spell.type == Spell::Type::SHIELD ? "SHIELD"
                                                            : "CONTROL");
        if (action.spell.type == Spell::Type::SHIELD || action.spell.type == Spell::Type::CONTROL) {
            _out << ' ' << action.spell.target_id;
        }
        if (action.spell.type == Spell::Type::WIND || action.spell.type == Spell::Type::CONTROL) {
            _out << ' ' << action.target;
        }
    } else if (action.type == Action::Type::MOVE) {
        _out << ' ' << action.target;
    }
    if (action.msg != "") {
        _out << ' ' << action.msg;
    }
    return _out;
}

void
output_actions(std::ostream& _out, std::vector<Action>& actions, const Game& game)
{
    for (int i = 0; i < 3; ++i) {
        game.normalize(actions[i].target);
        _out << actions[i] << std::endl;
    }
}

auto
main() -> int
{
    using std::cerr;
    using std::cin;
    using std::cout;
    using std::endl;

    Game game;
    game.init(cin);
    game.update(cin);

    Agent agent(game);

    std::vector<Action> actions;

    while (true) {
        agent.choose_actions(actions);
        output_actions(cout, actions, game);
        game.update(cin);
    }

    return 0;
}
