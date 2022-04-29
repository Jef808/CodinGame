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
enum class Corner : uint32_t
{
    NORTH_WEST = encode(0, 0),
    NORTH_EAST = encode(WIDTH - 1, 0),
    SOUTH_EAST = encode(WIDTH - 1, HEIGHT - 1),
    SOUTH_WEST = encode(0, HEIGHT - 1)
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

struct Direction
{
    double angle{ 0.0 };
    double normalizer{ -M_PIf64 };

    constexpr Direction() = default;

    explicit constexpr Direction(double angle_, double normalizer_ = -M_PIf64)
            : angle{ angle_ }
            , normalizer{ normalizer_ }
    {}

    /** Normalize the angle between (theta, theta + 2*PI] */
    static constexpr auto normalized(double angle_, const double theta) -> double
    {
        // (angle - theta) = new_angle + 2*pi*N with new_angle normalized
        double wrapped_angle = (angle_ - theta) * 0.5 * M_1_PI;  // N + new_angle / (2pi)

        // N
        int n_full_turns = std::floor(wrapped_angle);  // Round down so that real-part is positive (between 0 and 1)

        double new_angle = theta + 2.0 * M_PI * (wrapped_angle - n_full_turns);

        assert(new_angle > theta && new_angle < theta + 2.0 * M_PI + 0.000001);
        return new_angle;
    }

    static constexpr auto offset(const Direction& dir) -> Offset
    {
        constexpr std::array<Offset, 8> offsets_thresholds{ Offset::WEST,  Offset::SOUTH_WEST,
                                                            Offset::SOUTH, Offset::SOUTH_EAST,
                                                            Offset::EAST,  Offset::NORTH_EAST,
                                                            Offset::NORTH, Offset::NORTH_WEST };
        const double a = normalized(dir.angle, -M_PI);
        for (int i = 0; i < 8; ++i) {
            // if (a < -M_PI + (2 * i + 1) * M_PI / 8) {
            //     return offsets_thresholds[i];
            // }
            // Equivalent to -pi + pi/8 + i*(pi/4)
            if (a < (0.25*i - 0.875) * M_PI) {
                return offsets_thresholds[i];
            }
        }
        return Offset::WEST;
    }
};

struct Point
{
    int x;
    int y;

    constexpr Point()
            : x{ WIDTH }
            , y{ HEIGHT - 1 }
    {}
    constexpr Point(int x_, int y_)
            : x{ x_ }
            , y{ y_ }
    {}
    explicit constexpr Point(Offset off)
            : x{ get_x(off) }
            , y{ get_y(off) }
    {}

    constexpr auto operator!=(const Point& other) const noexcept -> bool
    {
        return this->x != other.x || this->y != other.y;
    }
    constexpr auto operator==(const Point& other) const noexcept -> bool
    {
        return this->x == other.x && this->y == other.y;
    }

    [[nodiscard]] inline auto constexpr direction_of(const Point& p, double normalization = -M_PI) const
      -> Direction
    {
        auto dx = static_cast<double>(p.x - this->x);
        auto dy = static_cast<double>(p.y - this->y);
        auto angle = Direction::normalized(std::atan2(-dy, dx), normalization);  // -dy because y-axis is "pointing downwards"
        return Direction{ angle };
    }
    [[nodiscard]] constexpr auto encoded(const Point& p) const noexcept -> uint32_t { return encode(p.x, p.y); }
};

inline auto constexpr
operator+(const Point& a, const Point& b) -> Point
{
    return { a.x + b.x, a.y + b.y };
}
inline auto constexpr
operator+(const Point& p, Offset off) -> Point
{
    return p + Point{ off };
}
inline auto constexpr
operator-(const Point& p) -> Point
{
    return Point{ -p.x, -p.y };
}
inline auto constexpr directed_floor(double x_, double y_, Direction dir) -> Point
{
    const auto off = Direction::offset(dir);
    Point p{};

    if (Horizontal(off) == Offset::EAST) {
        p.x = std::floor(x_);
    } else {
        p.x = std::ceil(x_);
    }

    if (Vertical(off) == Offset::SOUTH) {
        p.y = std::floor(y_);
    } else {
        p.y = std::ceil(y_);
    }

    return p;
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


struct Vector
{
    Point p;
    Direction dir;
    double norm;
};
inline auto constexpr
operator*(const Vector& v, double s) -> Vector
{
    return { v.p, v.dir, v.norm * s };
}
inline auto constexpr
operator-(const Point& a, const Point& b) -> Vector
{
    return { b, b.direction_of(a), distance(a, b) };
}
inline constexpr auto
Flow(const Vector& v) -> Vector
{
    double x = v.p.x + v.norm * std::cos(v.dir.angle);
    double y = v.p.y + v.norm * std::sin(v.dir.angle);
    return { directed_floor(x, y, v.dir), v.dir, v.norm };
}


} // namespace spring

#endif // TYPES_H_
