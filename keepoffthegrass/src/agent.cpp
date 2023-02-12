#include "agent.h"

#include <iostream>
#include <sstream>

#include "world.h"

using namespace std;

ostream& operator<<(ostream&, const Tile&);

void Agent::choose_actions(const WorldInfo& worldinfo) {
    for (auto _tile : worldinfo.my_tiles()) {
      const Tile& tile = _tile.get();
        if (tile.can_spawn) {
            int amount = 0; // TODO: pick amount of robots to spawn here
            if (amount > 0) {
                ostringstream action;
                action << "SPAWN " << amount << " " << tile;
                m_actions.emplace_back(action.str());
            }
        }
        if (tile.can_build) {
            bool should_build =
                    false; // TODO: pick whether to build recycler here
            if (should_build) {
                ostringstream action;
                action << "BUILD " << tile;
                m_actions.emplace_back(action.str());
            }
        }
    }

    for (Tile tile : worldinfo.my_units()) {
        bool should_move = false; // TODO: pick whether to move units from here
        if (should_move) {
            int amount = 0; // TODO: pick amount of units to move
            Tile target;    // TODO: pick a destination
            ostringstream action;
            action << "MOVE " << amount << " " << tile << " " << target;
            m_actions.emplace_back(action.str());
        }
    }
}

void Agent::output_actions() const {
    if (m_actions.empty()) {
        cout << "WAIT" << endl;
    } else {
        for (const std::string& action : m_actions) {
            cout << action << ";";
        }
        cout << endl;
    }
}
