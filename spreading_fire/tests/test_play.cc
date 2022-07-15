#include "game.h"
#include "viewer/viewer.h"

#include <SFML/Graphics/Text.hpp>
#include <cassert>
#include <fstream>
#include <string>


int main(int argc, char *argv[]) {

  // Create and initialize the game object
  Game game{};
  std::cerr << "Created a Game" << std::endl;
  {
    std::ifstream ifs{TEST_DATA_DIR "/input1.txt"};
    if (not ifs) {
      std::cerr << "Failed to open input file" << std::endl;
      return EXIT_FAILURE;
    }
    game.init_input(ifs);
  }

  // Create the sfml window and initialize the viewer
  Viewer viewer;
  if (not viewer.init(game)) {
    std::cerr << "Failed to initialize the viewer (Loading tilemap failed)" << std::endl;
    return EXIT_FAILURE;
  }
  sf::RenderWindow window(sf::VideoMode(viewer.window_width(), viewer.window_height()), "SFML window");

  std::string message = "Expanding Fire";
  sf::Text msg;
  sf::Text turn_msg;
  msg.setFont(viewer.font());
  msg.setCharacterSize(viewer.text_size());
  msg.setFillColor(sf::Color::Red);
  turn_msg.setFont(viewer.font());
  turn_msg.setCharacterSize(viewer.text_size());
  turn_msg.setFillColor(sf::Color::Red);

  bool is_game_over = false;


  // run the main loop
  while (window.isOpen())
  {
    // handle events
    sf::Event event;
    while (window.pollEvent(event))
    {
        switch (event.type) {
            case sf::Event::Closed: {
                window.close();
                break;
            }

            case sf::Event::EventType::KeyPressed:
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                    break;
                }
                if (event.key.code == sf::Keyboard::H)
                {
                    viewer.previous();
                    message = "PREVIOUS";
                    break;
                }
                if (event.key.code == sf::Keyboard::L)
                {
                    viewer.next();
                    message = "NEXT";
                    break;
                }
                if (event.key.code == sf::Keyboard::Space)
                {
                    game.apply(Move{ Move::Type::Wait, NULL_INDEX });
                    while (not game.is_ready()) {
                        game.apply(Move{ Move::Type::Wait, NULL_INDEX });
                    }
                    viewer.set_data(game);
                    message = "WAIT";
                    break;
                }

            case sf::Event::MouseButtonPressed:
                {
                    auto x = event.mouseButton.x / 64;
                    auto y = event.mouseButton.y / 64;

                    size_t index = x % game.width() + y * game.width();

                    // Check mouse click is in range
                    if (index < 0 || index >= game.size()) {
                        message = "Mouse out of range";
                        break;
                    }

                    // Check square is valid move
                    if (not game.is_flammable(index)) {
                        message = "Invalid move";
                        break;
                    }

                    // Apply move
                    Move move{ Move::Type::Cut, index };
                    game.apply(move);
                    while (not game.is_ready()) {
                        game.apply(Move{ Move::Type::Wait, NULL_INDEX });
                    }

                    // Add new game board to viewer's data
                    viewer.set_data(game);

                    message = "Cut " + std::to_string(x) + " " + std::to_string(y);

                    break;
                }

            default:
                break;
        }
    }

    if (game.is_terminal()) {
      message = "GAME OVER";
      is_game_over = true;
    }

    window.clear();

    // draw the game
    window.draw(viewer);

    // draw message
    sf::Vector2f msg_pos = viewer.get_text_pos();
    msg.setString(message);
    msg.setPosition(msg_pos);

    turn_msg.setString("Turn " + std::to_string(game.turn()));

    turn_msg.setPosition({msg_pos.x, msg_pos.y + viewer.text_size()});

    window.draw(msg);
    window.draw(turn_msg);

    window.display();
  }







  return EXIT_SUCCESS;
}
