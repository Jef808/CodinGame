#ifndef TYPES_H_
#define TYPES_H_

#include <cstdint>
#include <cstdlib>
#include <type_traits>

namespace tb {

constexpr size_t Max_length = 500;
constexpr size_t Max_depth = 50;
constexpr size_t Max_actions = 6;

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
    Known_loss = -10000,
    Zero = 0,
    Known_win = 10000,
    Infinite = 32000
};

/** Score for being at position p after t turns. */
inline Value Score(size_t p, size_t t) {
    return Value(p - (Max_length / Max_depth) * t);
}

inline Value operator-(const Value v) {
    return Value(-to_int(v));
}

inline bool operator<(const Value a, const Value b) {
    return to_int(a) < to_int(b);
}

} // namespace tb

#endif // TYPES_H_
