#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <iostream>
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
const double ANGLE_THRESHOLD = 2.0 * RADIUS * std::sin(DISTANCE_THRESHOLD / 2.0);
constexpr double EPSILON = 0.00001;
constexpr double MOUSE_SPEED = 10.0;

struct Point_Euc
{
    double x;
    double y;
    bool operator==(const Point_Euc& other);
};

struct Point_Radial
{
    double radius;
    double angle;
    bool operator==(const Point_Radial& other);
};

inline Point_Euc
rad2euc(const Point_Radial& point)
{
    return { point.radius * std::cos(point.angle), point.radius * std::sin(point.angle) };
}

inline Point_Radial
euc2rad(const Point_Euc& point)
{
    return { std::sqrt(point.x * point.x + point.y * point.y), std::atan2(point.y, point.x) };
}

struct PointI_Euc
{
    PointI_Euc() = default;

    PointI_Euc(int x_, int y_)
            : x{ x_ }
            , y{ y_ }
    {}

    explicit PointI_Euc(const Point_Euc p)
            : x{ static_cast<int>(p.x) }
            , y{ static_cast<int>(p.y) }
    {}

    PointI_Euc& operator=(const Point_Euc p) { return *this = PointI_Euc(p); }

    int x;
    int y;
    bool operator==(const PointI_Euc& other);
};

inline bool
PointI_Euc::operator==(const PointI_Euc& other)
{
    return x == other.x && y == other.y;
}
inline bool
Point_Euc::operator==(const Point_Euc& other)
{
    return std::abs(x - other.x) + std::abs(y - other.y) < 2 * EPSILON;
}
inline bool
Point_Radial::operator==(const Point_Radial& other)
{
    return std::abs(radius - other.radius) + std::abs((std::abs(angle) - std::abs(angle))) < 2 * EPSILON;
}

inline double
angle_between(const Point_Euc& a, const Point_Euc& b)
{
    double ax = a.x;
    double ay = a.y;
    double bx = b.x;
    double by = b.y;
    bool a_reversed = false;
    if (ax < 0.0f && bx < 0.0f) {
        ax *= -1;
        bx *= -1;
        a_reversed = true;
    }
    return (1.0 - 2.0 * a_reversed) * std::atan2(by * a.x - ay * b.x, b.x * a.x + by * ay);
}

/// Some convenience inline functions
template<typename PointT, typename OtherPointT>
inline double
distance2(const PointT& a, const OtherPointT& b)
{
    const Point_Euc* aa;
    const Point_Euc* bb;
    if constexpr (std::is_same_v<PointT, Point_Radial>) {
        Point_Euc aa_ = rad2euc(a);
        aa = &aa_;
    }
    if constexpr (std::is_same_v<OtherPointT, Point_Radial>) {
        Point_Euc bb_ = rad2euc(b);
        bb = &bb_;
    }
    if constexpr (std::is_same_v<PointT, Point_Euc>) {
        aa = &a;
    }
    if constexpr (std::is_same_v<OtherPointT, Point_Euc>) {
        bb = &b;
    }

    double dx = aa->x - bb->x;
    double dy = aa->y - bb->y;
    return dx * dx + dy * dy;
}

inline Point_Euc
operator+(const Point_Euc& a, const Point_Euc& b)
{
    return { a.x + b.x, a.y + b.y };
}

inline Point_Euc
operator-(const Point_Euc& a, const Point_Euc& b)
{
    return { a.x - b.x, a.y - b.y };
}

inline double
radius2(const Point_Euc& p)
{
    static const Point_Euc origin{ 0.0, 0.0 };
    return distance2(p, origin);
}

inline Point_Euc
rescale(const Point_Euc& p, const double factor)
{
    double scaled_x = p.x * factor;
    double scaled_y = p.y * factor;
    return { scaled_x, scaled_y };
}

} // namespace

#endif // UTILITIES_H_
