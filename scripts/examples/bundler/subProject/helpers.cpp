#include <iostream>

#include "helpers.h"

#if RUNNING_OFFLINE
#include <offline_from_helpers>
#endif

namespace helpers {

void help() {
  std::cout << "I HELP YOU!" << std::endl;
}

} // namespace helpers
