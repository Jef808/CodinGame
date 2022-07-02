#include <cmath>

struct Point {
  int x;
  int y;
};

inline Point operator+(const Point &a, const Point &b) {
  return {a.x + b.x, a.y + b.y};
}
inline Point operator-(const Point &a, const Point &b) {
  return {a.x - b.x, a.y - b.y};
}

inline int manhattan_distance(const Point &a, const Point &b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

inline int euclidean_distance_2(const Point &a, const Point &b) {
  return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}
