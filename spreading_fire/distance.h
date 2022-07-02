/**
 * Floodfill a 2D grid while storing parents and shortest distances.
 *
 * Used for pathfinding on a grid where you can only move in the four
 * directions WEST, NORTH, EAST, SOUTH.
 */
#ifndef DISTANCE_H_
#define DISTANCE_H_

#include <algorithm>
#include <cstdint>
#include <vector>

class GridDistance {
public:
  GridDistance(size_t width, size_t height);

  void set_source(const size_t source);

  /**
   * Simply return the distance instead of the path.
   *
   * A successfull call to @set_source() must have been placed
   * before this gets used.
   */
  int get_distance(const size_t target) const { return m_dist[target]; }

  /**
   * Compute the distance and find the quickest path
   * from @target to @source on a grid
   */
  std::vector<size_t> get_path(size_t target);

  template <typename InputIter>
  InputIter closest_element(InputIter beg, InputIter end);

  template <typename InputIter>
  void distance_sort(InputIter beg, InputIter end);

private:
  const size_t m_width;
  const size_t m_height;

  std::vector<int> m_dist;
  std::vector<size_t> m_parent;
  size_t m_source;

  //void reset_buffers();
};

template <typename InputIter>
inline InputIter GridDistance::closest_element(InputIter beg, InputIter end) {
  return std::min_element(
      beg, end, [&D = m_dist](auto a, auto b) { return D[a] < D[b]; });
}

template <typename InputIter>
void GridDistance::distance_sort(InputIter beg, InputIter end) {
  return std::sort(beg, end,
                   [&D = m_dist](auto a, auto b) { return D[a] < D[b]; });
}

#endif // DISTANCE_H_
