#ifndef __TYPES_H_
#define __TYPES_H_

#define FMT_ENABLED 1
#define RUNNING_OFFLINE FMT_ENABLED
#define EXTRACTING_ONLINE_DATA 0

#include <type_traits>

namespace dp {

template<typename Enum>
std::underlying_type_t<Enum>
constexpr inline to_int(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

constexpr int max_turns = 200;
constexpr int max_height = 15;
constexpr int max_width = 100;
constexpr int max_n_clones = 50;
constexpr int max_n_elevators = 100;
constexpr int max_time_ms = 100;

} // namespace dp

#endif
