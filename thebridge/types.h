#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>
#include <cstdlib>
#include <type_traits>

namespace tb {

constexpr size_t Max_length = 500;
constexpr size_t Max_depth = 50;
constexpr size_t Max_actions = 6;

enum class Cell { Bridge, Hole };
//typedef std::array<std::vector<Cell>, 4> Road;

enum class Action {
    None=0, Speed=1, Jump=2, Up=3, Down=4, Slow=5, Wait=6
};

inline bool operator!(const Action a) {
    return a == Action::None;
}

size_t inline to_int(Action a) { return static_cast<int>(a); }

template<typename Enum>
std::underlying_type_t<Enum> to_int(Enum e) {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

typedef uint32_t Key;

enum class Value : int {
    Known_loss = 10000,
    Known_win = 0,
    Max = 270,
    Infinite = 32000
};

enum class Cost : int {
    Known_loss = 10000,
    Known_win = 0,
    Max = 270,
    Infinite = 32000
};

inline Cost operator+(const Cost c, const int i) {
    return Cost(to_int(c) + i);
}

inline Cost operator+(const Cost c, const Cost i) {
    return c + to_int(i);
}

inline Cost& operator+=(Cost& c, const Cost i) {
    return c = c + i;
}

inline Cost& operator+=(Cost& c, const int i) {
    return c = c + i;
}

inline Value operator-(const Value v) {
    return Value(-to_int(v));
}

inline bool operator<(const Value a, const Value b) {
    return to_int(a) < to_int(b);
}

} // namespace tb

#endif // TYPES_H_
