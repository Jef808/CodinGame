#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

#include "grid/grid.h"
#include "grid/bfs.h"

#include "helpers.h"

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

TEST_CASE( "Bfs gives the correct distance field", "[bfs]" ) {
  Grid<Tile> grid;
  std::vector<int> distance_field;

  SECTION( "On a grid without blocked tiles" ) {
    grid.set_dimensions(4, 3);
    std::vector<Tile> tiles = {
      F, F, F, F,
      F, F, F, F,
      F, F, F, F,
    };
    grid.set_tiles(std::move(tiles));

    SECTION( "From a corner" ) {
      bfs(grid, 3, distance_field);
      std::vector<int> expected{
        3, 2, 1, 0,
        4, 3, 2, 1,
        5, 4, 3, 2,
      };
      INFO(error_msg_distance_fields(4, 3, distance_field, expected));
      REQUIRE_THAT( distance_field, Equals(expected) );
    }

    SECTION( "From a center tile" ) {
      bfs(grid, 5, distance_field);
      std::vector<int> expected{
        2, 1, 2, 3,
        1, 0, 1, 2,
        2, 1, 2, 3,
      };
      INFO(error_msg_distance_fields(4, 3, distance_field, expected));
      REQUIRE_THAT( distance_field, Equals(expected) );
    }
  }

  SECTION( "On a grid with blocked tiles" ) {
    grid.set_dimensions(6, 7);
    std::vector<Tile> tiles = {
      B, B, F, F, B, F,
      F, F, F, F, B, F,
      F, F, F, F, F, F,
      F, B, F, F, F, F,
      F, B, B, F, F, F,
      F, B, F, F, F, F,
      F, B, F, F, F, F,
    };
    grid.set_tiles(std::move(tiles));

    const int X = CG::INT::INFTY;

    SECTION( "From an unblocked tile" ) {
      bfs(grid, 32, distance_field);
      std::vector<int> expected = {
        X, X, 7, 6, X, 8,
        8, 7, 6, 5, X, 7,
        7, 6, 5, 4, 5, 6,
        8, X, 4, 3, 4, 5,
        9, X, X, 2, 3, 4,
        10,X, 0, 1, 2, 3,
        11,X, 1, 2, 3, 4,
      };
      INFO(error_msg_distance_fields(6, 7, distance_field, expected));
      REQUIRE_THAT( distance_field, Equals(expected) );
    }

    SECTION( "From a blocked tile" ) {
      bfs(grid, 4, distance_field);
      std::vector<int> expected = {
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
      };
      INFO(error_msg_distance_fields(6, 7, distance_field, expected));
      REQUIRE_THAT( distance_field, Equals(expected) );
    }

    SECTION( "From multiple sources" ) {
      std::vector<size_t> sources{
        grid.index_of(0, 1),
        grid.index_of(4, 3)
      };
      std::vector<int> expected = {
        X, X, 3, 4, X, 4,
        0, 1, 2, 3, X, 3,
        1, 2, 3, 2, 1, 2,
        2, X, 2, 1, 0, 1,
        3, X, X, 2, 1, 2,
        4, X, 4, 3, 2, 3,
        5, X, 5, 4, 3, 4,
      };

      bfs(grid,
          sources.begin(),
          sources.end(),
          distance_field);

      INFO(error_msg_distance_fields(6, 7, distance_field, expected));
      REQUIRE_THAT( distance_field, Equals(expected) );
    }

    SECTION( "The overload with a partially prepopulated distance field" ) {
      const auto U = CG::INT::UNVISITED;

      std::vector<int> output_distance_field = {
        X, X, U, U, X, U,
        0, 1, U, U, X, U,
        1, U, U, U, 1, U,
        U, X, U, 1, 0, 1,
        U, X, X, U, 1, U,
        U, X, U, U, U, U,
        U, X, U, U, U, U,
      };
      bfs(grid, output_distance_field);
      std::vector<int> expected = {
        X, X, 3, 4, X, 4,
        0, 1, 2, 3, X, 3,
        1, 2, 3, 2, 1, 2,
        2, X, 2, 1, 0, 1,
        3, X, X, 2, 1, 2,
        4, X, 4, 3, 2, 3,
        5, X, 5, 4, 3, 4,
      };
      INFO(error_msg_distance_fields(6, 7, output_distance_field, expected));
      REQUIRE_THAT( distance_field, Equals(expected) );
    }
  }
}
