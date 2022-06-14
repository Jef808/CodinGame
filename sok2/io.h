#ifndef IO_H_
#define IO_H_

#include "types.h"

#include <iostream>

namespace sok2 {


inline std::istream& operator>>(std::istream& is, Window& w) {
    return is >> w.x >> w.y;
}

inline std::istream& operator>>(std::istream& is, Building& b) {
    return is >> b.width >> b.height;
}

inline std::istream& operator>>(std::istream& is, Game& g) {
    return is >> g.building
              >> g.jumps
              >> g.current_pos;
}

inline std::ostream& operator<<(std::ostream& os, Window& w) {
    return os << w.x << ' ' << w.y;
}


} // namespace sok2

#endif // IO_H_
