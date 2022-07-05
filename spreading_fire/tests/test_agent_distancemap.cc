#include "agent.h"
#include "game.h"
#include "viewer/viewer.h"

#include <SFML/Graphics.hpp>

#include <cassert>
#include <fstream>
#include <iostream>


const sf::RectangleShape
Square(sf::Vector2f(64.0, 64.0));

std::array<sf::Color, 5>
palette {
  sf::Color::Yellow,
  sf::Color::Blue,
  sf::Color::Green,
  sf::Color::Red,
  sf::Color::Cyan
};


// Generate data for viewing the distance function
std::vector<std::pair<sf::RectangleShape, sf::Text>>
distance_map_data(
  const std::vector<int>& distances,
  const std::vector<sf::Text>& numbers,
  size_t width,
  size_t height,
  int fire_duration,
  int max_distance)
{
  std::vector<std::pair<sf::RectangleShape, sf::Text>> ret;

  for (size_t j = 0; j < height; ++j)
    for (size_t i = 0; i < width; ++i) {

      // Normalize the distances so that they align with the
      // color palette (which are separated by unit distance)
      int d = distances[i + j * width];
      int d_normal = d / fire_duration;

      auto& [r, t] = ret.emplace_back(Square, numbers[d]);
      r.setPosition(64.0 * i, 64.0 * j);

      if (d > 1000) {
        r.setFillColor(sf::Color::Black);
      }
      else if (d_normal > max_distance) {
        r.setFillColor(sf::Color::White);
      }
      else {
        r.setFillColor(palette[d_normal % 5]);
      }
      t.setPosition(sf::Vector2f(64.0 * i + 32 - 15, 64.0 * j + 32 - 15));
    }

  return ret;
}


int main(int argc, char *argv[]) {

  std::ifstream ifs{TEST_DATA_DIR "/input1.txt"};
  if (not ifs) {
    std::cerr << "Failed to open input file" << std::endl;
    return EXIT_FAILURE;
  }
  Game game{};
  game.init_input(ifs);

  Agent agent{game};

  // Initialize the viewer
  sf::RenderWindow window(sf::VideoMode(512, 512), "Distance map");
  Viewer viewer;
  if (not viewer.init(game)) {
    std::cerr << "Failed to initialize viewer" << std::endl;
    return EXIT_FAILURE;
  }
  // Load a font
  sf::Font font;
  font.loadFromFile(VIEWER_DATA_DIR "/JetBrainsMono-Bold.ttf");
  std::vector<sf::Text> text_distances;

  // Generate the distance map
  agent.generate_distance_map();
  const auto &dists = agent.view_distance_map();


  // Get the largest non-infinite distance
  const int largest_dist = *std::max_element(dists.begin(), dists.end(), [](auto a, auto b) {
    if (a > 1000) {
      return false;
    }
    else if (b > 1000) {
      return true;
    }
    return a < b;
  });
  const int FD = game.duration_fire(game.fire_origin());


  // Create SFML text objects for each distances
  for (size_t i = 0; i <= largest_dist; ++i) {
    sf::Text& text = text_distances.emplace_back(std::to_string(i), font);
    text.setCharacterSize(30);
    text.setStyle(sf::Text::Bold);
    text.setFillColor(sf::Color::Black);
  }


  // The data used to display the function map:
  // A rectangle to be colored along with the distance
  std::vector<std::pair<sf::RectangleShape, sf::Text>> dmap_data;

  // Variables to keep track of when recomputing the viewing data
  // is needed
  int max_dist = 1;
  bool changed = true;

  // false means we see the normal grid, true we see the distance function
  bool toggle_grid_distance_view = false;




  // run the main loop
  while (window.isOpen()) {

    // handle events
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }

      if (event.type == sf::Event::EventType::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
          window.close();
        }
        // K to up the max distance colored
        else if (event.key.code == sf::Keyboard::K) {
          int old_max = max_dist;
          max_dist = std::min(largest_dist, max_dist+1);
          changed = max_dist != old_max;
        }
        // J to lower it
        else if (event.key.code == sf::Keyboard::J) {
          int old_max = max_dist;
          max_dist = std::max(1, max_dist-1);
          changed = max_dist != old_max;
        }
        // Space to toggle between the two views
        else if (event.key.code == sf::Keyboard::Space) {
          toggle_grid_distance_view = !toggle_grid_distance_view;
        }
      }
    }

    // Regenerate the distance map data if needed
    if (toggle_grid_distance_view && changed) {
      dmap_data = distance_map_data(dists, text_distances, game.width(), game.height(), FD, max_dist);
      changed = false;
    }

    window.clear();

    // Draw the distance map data
    if (toggle_grid_distance_view) {
      for (const auto& [r, t] : dmap_data) {
        window.draw(r);
        window.draw(t);
      }
    }
    // or draw the grid
    else {
      window.draw(viewer);
    }

    window.display();
  }

  return EXIT_SUCCESS;
}
