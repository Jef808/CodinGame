#ifndef UTILITIES_H_
#define UTILITIES_H_


#include <cmath>

namespace escape_cat {

/**
 * type alias to make function calls less confusing.
 *
 * They both are units of distance measurements.
 * Radian is only useful for distances along circle arcs, but it is scale-invariant.
 * NPixel can be thought of as regular euclidean distance, but I chose to emphasis that name
 * because (at least in vertical or horizontal straight lines), to measure it you count the number
 * of lattice points encoutered along a path. In particular it is not scale invariant and depends
 * on extraneous, error-inducing pieces of information.
 */
using Radian = double;
using NPixel = double;

constexpr int RADIUS = 500;
constexpr int RADIUS2 = 500 * 500;
constexpr double DISTANCE_THRESHOLD = 80.0;
constexpr double EPSILON = 0.00001;

/**
 * All implementations of == and != for double-valued points simply
 * round the doubles down to ints then compare.
 * This is all what's needed for this project but I put a little warning to cerr
 * in there just to be sure I don't forget!
 *
 * In particular, it could be that p1 == p2 yet distance(p1, p2) != 0
 */
struct PointI {
    using value_type = int;
    int x;
    int y;
    bool operator!=(const PointI& other);
};
struct Point {
    using value_type = double;
    double x;
    double y;
    operator PointI();
    bool operator!=(const Point& other);
};


inline Point::operator PointI() {
    PointI p;
    p.x = static_cast<int>(x);
    p.y = static_cast<int>(y);
    return p;
}

inline bool PointI::operator!=(const PointI& other) { return x != other.x || y != other.y; }


/// Some convenience inline functions
template<typename PointT, typename OtherPointT>
inline double distance2(const PointT& a, const OtherPointT& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return dx * dx + dy * dy;
}

inline Point operator+(const Point& a, const Point& b) {
    return { a.x + b.x, a.y + b.y };
}

inline Point operator-(const Point& a, const Point& b) {
    return { a.x - b.x, a.y - b.y };
}

inline double radius2(const Point& p) {
    static const Point origin{ 0.0, 0.0 };
    return distance2(p, origin);
}

inline Point rescale(const Point& p, const double factor) {
    double scaled_x = p.x * factor;
    double scaled_y = p.y * factor;
    return { scaled_x, scaled_y };
}

/**
 * Apply a rotation of angle @theta to the point
 */
inline Point rotate(const Point& p, const double angle) {
    double cos = std::cos(angle);
    double sin = std::sin(angle);
    double x = cos * p.x - sin * p.y;
    double y = sin * p.x + cos * p.y;
    return { x, y };
}


} //namespace



#endif // UTILITIES_H_
