#ifndef IO_H_
#define IO_H_

#include "types.h"

#include <cassert>
#include <iostream>

namespace sok2 {


inline std::istream& operator>>(std::istream& is, Building& b) {
    return is >> b.width >> b.height;
}

inline std::istream& operator>>(std::istream& is, Window& w) {
    return is >> w.x >> w.y;
}

inline std::istream& operator>>(std::istream& is, Game& g) {
    return is >> g.building
              >> g.turns_left
              >> g.current_pos;
}

inline std::ostream& operator<<(std::ostream& os, Window& w) {
    return os << w.x << ' ' << w.y;
}

inline std::ostream& operator<<(std::ostream& os, Heat& h) {
  switch (h) {
    case Heat::cold:    return os << "COLDER";
    case Heat::warm:    return os << "WARMER";
    case Heat::neutral: return os << "SAME";
    default:
      assert(false && "Invalid Heat value while outputting to ostream");
  }
}

inline std::istream& operator>>(std::istream& is, Heat& h) {
  char c;
  is >> c;
  is.ignore();

  switch (c) {
    case 'W': h = Heat::warm;    break;
    case 'C': h = Heat::cold;    break;
    case 'S': h = Heat::neutral; break;
  }

  return is;
}

} // namespace sok2

#endif // IO_H_
