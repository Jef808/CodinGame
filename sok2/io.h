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

inline std::ostream& operator<<(std::ostream& os, const Window& w) {
    return os << w.x << ' ' << w.y;
}

inline std::ostream& operator<<(std::ostream& os, const Heat& h) {
  switch (h) {
    case Heat::cold:    return os << "COLDER";
    case Heat::warm:    return os << "WARMER";
    case Heat::neutral: return os << "SAME";
    case Heat::unknown: return os << "UNKNOWN";
    default:
      assert(false && "Invalid Heat value while outputting to ostream");
  }
}

inline std::istream& operator>>(std::istream& is, Heat& h) {
  static std::string buf;
  is >> buf;

  switch (buf[0]) {
    case 'W': h = Heat::warm;    break;
    case 'C': h = Heat::cold;    break;
    case 'S': h = Heat::neutral; break;
    case 'U': h = Heat::unknown; break;
    default:
      assert(false && "Invalid Heat value fed to istream");
  }

  return is;
}

} // namespace sok2

#endif // IO_H_
