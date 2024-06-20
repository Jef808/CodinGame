#ifndef __AGENT_H_
#define __AGENT_H_

#include "olymbits.h"

#include <iosfwd>

namespace olymbits {

std::array<int, 4> get_action_weights(const Olymbits& game);

} // namespace tb

#endif // __AGENT_H_
