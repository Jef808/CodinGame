#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

#include "grid/grid.h"

#include <sstream>

using namespace Catch::Matchers;

using namespace CG;

namespace {

struct Tile {
  enum class Type {
    Free, Blocked
  };
  int x;
  int y;
  Type type;
  [[nodiscard]] bool is_blocked(int distance = 0) const {
    return type == Type::Blocked;
  }
  bool operator==(const Tile& other) const { return x == other.x && y == other.y && type == other.type; }
  bool operator!=(const Tile& other) const { return x != other.x || y != other.y || type != other.type; }
};

static const Tile F = {0, 0, Tile::Type::Free};    // Free
static const Tile B = {0, 0, Tile::Type::Blocked}; // Blocked

} // namespace


using Index = Grid<Tile>::index_type;

TEST_CASE( "Grid is instantiated correctly", "[grid]" ) {
  Grid<Tile> grid{4, 4};

  REQUIRE( grid.width() == 4 );
  REQUIRE( grid.height() == 4 );

  SECTION( "Neighbours get calculated correctly" ) {
    REQUIRE_THAT( grid.neighbours_of(0), UnorderedEquals(std::vector<Index>{1, 4}) );
    REQUIRE_THAT( grid.neighbours_of(3), UnorderedEquals(std::vector<Index>{2, 7}) );
    REQUIRE_THAT( grid.neighbours_of(12), UnorderedEquals(std::vector<Index>{8, 13}) );
    REQUIRE_THAT( grid.neighbours_of(15), UnorderedEquals(std::vector<Index>{11, 14}) );

    REQUIRE_THAT( grid.neighbours_of(2), UnorderedEquals(std::vector<Index>{1, 3, 6}) );
    REQUIRE_THAT( grid.neighbours_of(4), UnorderedEquals(std::vector<Index>{0, 5, 8}) );
    REQUIRE_THAT( grid.neighbours_of(7), UnorderedEquals(std::vector<Index>{3, 6, 11}) );
    REQUIRE_THAT( grid.neighbours_of(14), UnorderedEquals(std::vector<Index>{10, 13, 15}) );

    REQUIRE_THAT( grid.neighbours_of(5), UnorderedEquals(std::vector<Index>{1, 4, 6, 9}) );
    REQUIRE_THAT( grid.neighbours_of(9), UnorderedEquals(std::vector<Index>{5, 8, 10, 13}) );
  }

  SECTION( "Tiles get set correctly with the set_tiles method" ) {
    const std::vector<Tile> tiles = {
      B, B, F, F,
      F, F, F, F,
      F, F, F, F,
      F, B, F, F,
    };
    grid.set_tiles(std::vector<Tile>(tiles.begin(), tiles.end()));
    REQUIRE_THAT(std::vector<Tile>(grid.begin(), grid.end()), Equals(tiles));
  }
}
