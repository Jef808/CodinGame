#ifndef TYPES_H_
#define TYPES_H_

#include <array>
#include <type_traits>
#include <string_view>
#include <iosfwd>
#include <cmath>
#include <cassert>

namespace spring {

constexpr int WIDTH = 0x44DF;
constexpr int HEIGHT = 0x2329;
constexpr int MAX_MOVE = 800;
constexpr int MAX_TURNS = 220;
constexpr double MM_PI_12 = 0.26179938779914940782944655;
constexpr double MM_PI_4 = 0.78539816339744827899949087;
constexpr double MM_5PI_12 = 1.30899693899574720568068642;

namespace details {

template<typename E,
         typename U = std::conditional_t<std::is_enum_v<E>,
                                         std::underlying_type_t<E>,
                                         std::conditional_t<std::is_integral_v<E>, E, void>>>
constexpr auto
to_integral(E e) -> U
{
    return static_cast<U>(e);
}

} // namespace details

inline constexpr auto
encode(int x, int y) -> uint32_t
{
    return ((unsigned)(y & 0xFFFF) << 16) | (x & 0xFFFF);
}
enum class Offset : uint32_t
{
    NONE = encode(0, 0),
    EAST = encode(1, 0),
    WEST = encode(-1, 0),
    SOUTH = encode(0, 1),
    NORTH = encode(0, -1),
    SOUTH_EAST = SOUTH | EAST,
    SOUTH_WEST = SOUTH | WEST,
    NORTH_EAST = NORTH | EAST,
    NORTH_WEST = NORTH | WEST
};

inline auto constexpr get_x(Offset off) -> int
{
    return static_cast<int16_t>(details::to_integral(off) & 0xFFFF);
}
inline auto constexpr get_y(Offset off) -> int
{
    return static_cast<int16_t>(details::to_integral(off) >> 16);
}
inline auto constexpr Vertical(Offset off) -> Offset
{
    return Offset(encode(0, get_y(off)));
}
inline auto constexpr Horizontal(Offset off) -> Offset
{
    return Offset(encode(get_x(off), 0));
}

struct Point
{
    int x;
    int y;

    constexpr auto operator!=(const Point& other) const noexcept -> bool
    {
        return this->x != other.x || this->y != other.y;
    }
    template<typename Other>
    constexpr auto operator==(const Other& other) const noexcept -> bool
    {
        return this->x == other.x && this->y == other.y;
    }
    template<typename Offset>
    inline auto constexpr operator+=(const Offset& offset) -> Point&
    {
        this->x += offset.x;
        this->y += offset.y;
        return *this;
    }
    [[nodiscard]] constexpr auto encoded(const Point& p) const noexcept -> uint32_t
    {
        return encode(p.x, p.y);
    }
};

inline auto constexpr
operator+(const Point& a, const Point& b) -> Point
{
    return { a.x + b.x, a.y + b.y };
}
inline auto constexpr
operator-(const Point& p) -> Point
{
    return Point{ -p.x, -p.y };
}

inline auto constexpr
operator*(const Point& p, double d) -> Point
{
    Point pd{};
    double px = p.x * d;
    double py = p.y * d;
    if (px > WIDTH / 2.0) {
        if (d > 1.0) {
            pd.x = std::ceil(px);
        } else {
            pd.x = std::floor(px);
        }
    } else {
        if (d > 1.0) {
            pd.x = std::floor(px);
        } else {
            pd.x = std::ceil(px);
        }
    }
    if (py > HEIGHT / 2.0) {
        if (d > 1.0) {
            pd.y = std::ceil(py);
        } else {
            pd.y = std::floor(py);
        }
    } else {
        if (d > 1.0) {
            pd.y = std::floor(py);
        } else {
            pd.y = std::ceil(py);
        }
    }
    return pd;
}

inline auto constexpr dot(const Point& a, const Point& b) -> double
{
    return a.x * b.x + a.y * b.y;
}
inline auto constexpr distance2(const Point& a, const Point& b) -> int
{
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}
inline auto constexpr distance(const Point& a, const Point& b) -> double
{
    return std::sqrt(distance2(a, b));
}
inline constexpr auto
norm(const Point& p) -> double
{
    constexpr Point O{ 0, 0 };
    return distance(p, O);
}

inline constexpr auto
manhattan_norm(const Point& p) -> int
{
    return std::abs(p.x) + std::abs(p.y);
}

inline constexpr auto
manhattan_dist(const Point& a, const Point& b) -> int
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

struct Vector
{
    Point p;
    double x;
    double y;
    [[nodiscard]] auto target() const noexcept -> Point {
        Point ret;
        if (p.x + x > WIDTH / 2.0) {
            ret.x = std::ceil(p.x + x);
        } else {
            ret.x = std::floor(p.x + x);
        }
        if (p.y + y > HEIGHT / 2.0) {
            ret.y = std::ceil(p.y + y);
        } else {
            ret.y = std::floor(p.y + y);
        }
        return ret;
    }
};

inline auto constexpr
operator*(const Vector& v, double s) -> Vector
{
    return { v.p, v.x * s, v.y * s };
}

inline auto constexpr
operator-(const Point& a, const Point& b) -> Vector
{
    return { b, double(a.x - b.x), double(a.y - b.y) };
}

inline constexpr auto
Unit(const Point& p) -> Vector
{
    double d = norm(p);
    return { p, p.x/d, p.y/d };
}
constexpr const Point Point_None{ WIDTH, HEIGHT - 1 };

struct Segment {
    Point start;
    Point end;
};

} // namespace spring

#endif // TYPES_H_
