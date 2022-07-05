#include "game.h"
#include "agent.h"

#include "viewer/viewer.h"


#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>





void step(Game& game, Agent& agent) {
  Move move = agent.choose_move();

  std::cout << "Got Move\n";

  if (move.type == Move::Type::Wait) {
    std::cout << "WAIT" << std::endl;
  } else {
    auto [x, y] = game.coords(move.index);
    std::cout << x << ' ' << y << std::endl;
  }

  game.apply(move);
}



int main(int argc, char *argv[]) {

  // Create game and the agent
  Game game{};
  std::cerr << "Created a Game" << std::endl;
  {
    std::ifstream ifs { TEST_DATA_DIR "/input1.txt" };
    if (not ifs) {
        std::cerr << "Failed to open input file" << std::endl;
        return EXIT_FAILURE;
    }
    game.init_input(ifs);
    std::cerr << "Initialized Game" << std::endl;
  }
  Agent agent{game};
  std::cerr << "Created an agent" << std::endl;




  // Create the sfml window
  sf::RenderWindow window(sf::VideoMode(512, 512), "SFML window");


  // Initialize the viewer
  Viewer viewer;
  std::cerr << "Created a viewer" << std::endl;

  if (not viewer.init(game)) {
    std::cerr << "Failed to initialize the viewer (Loading tilemap failed)" << std::endl;
    return EXIT_FAILURE;
  }
  std::cerr << "Initialized the viewer" << std::endl;




  bool should_step_game = true;

  // run the main loop
  while (window.isOpen())
  {
    if (should_step_game) {
      step(game, agent);
      viewer.append_data(game);
      should_step_game = false;
    }

    // handle events
    sf::Event event;
    while (window.pollEvent(event))
    {
      if(event.type == sf::Event::Closed)
      {
        window.close();
      }

      if (event.type == sf::Event::EventType::KeyPressed)
      {
        if (event.key.code == sf::Keyboard::Escape)
        {
          window.close();
        }
        if (event.key.code == sf::Keyboard::Space)
        {
          should_step_game = true;
        }
      }

      // etc...

    }

    // draw the game
    window.clear();
    window.draw(viewer);
    window.display();
  }
    return EXIT_SUCCESS;
}
