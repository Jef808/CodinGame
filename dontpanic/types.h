#ifndef __TYPES_H_
#define __TYPES_H_

#include <type_traits>

template<typename Enum>
std::underlying_type_t<Enum>
constexpr inline to_int(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

namespace dp {

constexpr int max_turns = 200;
constexpr int max_height = 15;
constexpr int max_width = 100;
constexpr int max_n_clones = 50;
constexpr int max_n_elevators = 100;
constexpr int max_time_ms = 100;

} // namespace dp

#endif
