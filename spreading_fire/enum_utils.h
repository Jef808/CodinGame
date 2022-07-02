#ifndef ENUM_UTILS_H_
#define ENUM_UTILS_H_

#include <type_traits>

template <typename E, typename I = std::enable_if_t<std::is_enum_v<E>,
                                                    std::underlying_type_t<E>>>
I to_int(E e) {
  return static_cast<I>(e);
}

#endif // ENUM_UTILS_H_
