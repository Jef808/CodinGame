#include <cassert>
#include <cmath>
#include <complex>
#include <iostream>

#include "../catandmouse.h"

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

constexpr double EPSILON = 0.0001;

using namespace Catch::Matchers;
using namespace EscapeTheCat;

TEST_CASE( "Dummy test", "[dummy]" ) {
  REQUIRE_THAT(0.0, WithinAbs(0.0, EPSILON));
}
