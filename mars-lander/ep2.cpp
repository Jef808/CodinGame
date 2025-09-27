#include <vector>
#include <iostream>

constexpr float WIDTH = 7000.0f;
constexpr float G = 3.711f;
constexpr float MAX_LANDING_SPEED_X = 40.0f;
constexpr float MAX_LANDING_SPEED_Y = 20.0f;

struct Point {
  float x;
  float y;
};

using Segment = std::pair<Point, Point>;

struct Lander {
  Point position;
  Point speed;
  float fuel;
  float angle;
  float power;
};

std::vector<Segment> read_map() {
  std::vector<Segment> segments;
  std::vector<Point> points;

  int n;
  std::cin >> n;
  std::cin.ignore();

  points.resize(n);
  segments.reserve(n - 1);

  for (int i = 0; i < n; ++i) {
    auto& [x, y] = points[i];
    std::cin >> x >> y;
    std::cin.ignore();

    if (i > 0) {
      segments.emplace_back(points[i - 1], points[i]);
    }
  }

  return segments;
}

Lander read_lander() {
  Lander lander;
  std::cin >> lander.position.x
           >> lander.position.y
           >> lander.speed.x
           >> lander.speed.y
           >> lander.fuel
           >> lander.angle
           >> lander.power;
  std::cin.ignore();

  return lander;
}

int main() {
  auto segments = read_map();

  for (;;) {
    auto lander = read_lander();

    std::cout << "-20 3" << std::endl;
  }

  return 0;
}
