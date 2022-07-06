#include "game.h"
#include "viewer/viewer.h"

#include <deque>
#include <fstream>
#include <iostream>

const int width = 8;
const int height = 8;

std::vector<int> distances;
std::deque<size_t> boundary;
std::vector<int> seen;

struct EdgeCost {
    const Game& game;

    EdgeCost(const Game& _game)
        : game{ _game }
    {}

    constexpr static int Zero = 0;
    constexpr static int One  = 1;
    constexpr static int Infinite = 32000;

    int operator()(size_t source, size_t target) {
        if (not game.is_flammable(target)) {
            return Infinite;
        }
        return game.duration_fire(source);
    }
};


void generate_distance_map(const Game &game) {

  enum Direction {
    WEST = 1,     NORTH = width,
    EAST = -WEST, SOUTH = -NORTH,
  };

  EdgeCost edge_cost(game);

  seen.resize(game.size(), 0);
  distances[game.fire_origin()] = 0;
  boundary.push_back(game.fire_origin());

  while (not boundary.empty()) {
    size_t current = boundary.front();
    boundary.pop_front();
    const int distance = distances[current];

    for (Direction d : {EAST, NORTH, WEST, SOUTH}) {
      const size_t nbh = current + d;

      // Avoid interior points
      if (seen[nbh] > 0) {
        continue;
      }

      const int this_distance = distance + edge_cost(current, nbh);

      // Mark points which are not flammable as seen and proceed to next nbh
      if (this_distance >= EdgeCost::Infinite) {
        seen[nbh] = 1;
        continue;
      }

      // Otherwise, add nbh to the end of the queue and update stats
      boundary.push_back(nbh);

      m_parents[nbh][-d] = this_distance;

      if (this_distance < distances[nbh]) {
        distances[nbh] = this_distance;
      }
    }

    // Add to seen set once a square's four neighbours are processed
    seen[current] = 1;
  }
}

int main(int argc, char *argv[]) {

  std::ifstream ifs{TEST_DATA_DIR "/input1.txt"};
  if (not ifs) {
    std::cerr << "Failed to open input file" << std::endl;
    return EXIT_FAILURE;
  }
  Game game{};
  game.init_input(ifs);

  sf::RenderWindow window(sf::VideoMode(512, 512), "Distance map");
  Viewer viewer;
  if (not viewer.init(game)) {
    std::cerr << "Failed to initialize viewer" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
