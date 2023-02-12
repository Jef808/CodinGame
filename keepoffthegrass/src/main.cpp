#include <iostream>
#include <sstream>

#include "world.h"
#include "worldinfo.h"
#include "agent.h"

using namespace std;

std::ostream& operator<<(std::ostream& ioOut, const Tile& obj) {
    return obj.dump(ioOut);
}


int main(int argc, char* argv[]) {

    World world;
    WorldInfo worldinfo{world};
    Agent agent;

    cerr << "Entering main loop" << endl;
    while (true) {
        worldinfo.update();
        cerr << "Updated worldinfo" << endl;
        agent.choose_actions(worldinfo);
        cerr << "chose actions" << endl;
        agent.output_actions();
    }

    return 0;
}
