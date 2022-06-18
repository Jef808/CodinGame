#ifndef UTILS_H_
#define UTILS_H_

#include "types.h"

#include <algorithm>
#include <cmath>
#include <type_traits>

namespace sok2 {


/**
 * Compute the squared distance between two windows.
 *
 * NOTE: The components are bounded from above by 1e+04,
 * so distance_squared is bounded from above by 8e+08 and
 * there it is safe to use `ints' if we don't further
 * multiply the result.
 */
inline int distance_squared(const Point& a, const Point& b) {
    return (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y);
}

/**
 * Compute the euclidean distance between two points.
 */
inline double distance(const Point& a, const Point& b) {
    return std::sqrt(distance_squared(a, b));
}

inline Heat compare(int hot, int cold) {
    return hot < cold ? Heat::warm
        : hot > cold ? Heat::cold
        : Heat::neutral;
}

/**
 * Compare the distance between a point and two other points.
 *
 * By only using ints and distance_squared, the result is exact
 * for Points with integer components, but those components are
 * restricted to be <= 1e+04
 */
inline Heat compare_distances(const Point& p, const Point& hot, const Point& cold) {
    return compare(distance_squared(p, hot), distance_squared(p, cold));
}

/**
 * Return the point opposite to p with respect to the
 * segment [m, M] sitting inside of [0, Max], making
 * sure to leave an odd integral distance between p and
 * the result, to avoid missing a lucky equal distance.
 *
 * @Param m  The lower bound of the interval
 * @Param M  The upper bound of the interval
 * @Param Max  The global width/height of the bounding grid
 * @Param p  The point to reflect
 *
 * @Note  Midpoint is m + M - p
 */
inline int opposite(int m, int M, int Max, int p) {
    return std::clamp(m + M - p + ((m + M) & 1) * (1 - 2 * (p & 1)), 0, Max);
}

inline bool operator==(const Point& a, const Point& b) {
    return a.x == b.x && a.y == b.y;
}

inline bool operator!=(const Point& a, const Point& b) {
    return a.x != b.x || a.y != b.y;
}

inline int n_windows(const Game& game) {
    return game.building.height * game.building.width;
}

} // namespace sok2

#endif // UTILS_H_
