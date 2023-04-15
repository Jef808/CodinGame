#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

#include "grid/grid.h"
#include "grid/bfs.h"

#include <sstream>

namespace {

enum Tile {
  F, // Free
  B  // Blocked
};

using Grid = CG::Grid<Tile>;
using Index = typename Grid::index_type;

} // namespace

using namespace Catch::Matchers;

TEST_CASE( "Grid is instantiated correctly", "[grid]" ) {
  Grid grid{4, 4};

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
    std::vector<Tile> tiles = {
      B, B, F, F,
      F, F, F, F,
      F, F, F, F,
      F, B, F, F,
    };
    grid.set_tiles(std::vector<Tile>(tiles.begin(), tiles.end()));
    REQUIRE_THAT( std::vector<Tile>(grid.begin(), grid.end()), Equals(tiles) );
  }
}

std::string error_msg_distance_fields(int width,
                                      int height,
                                      const std::vector<int>& expected,
                                      const std::vector<int>& actual) {
  std::stringstream ss;
  ss << "Expected\n";
  for (auto y = 0; y < height; ++y) {
    for (auto x = 0; x < width; ++x) {
      const auto value = expected[x + width * y];
      ss << (value == CG::INFTY ? "X " : std::to_string(value))
         << (0 <= value && value < 10 ? "  " : " ");
    }
    ss << '\n';
  }
  ss << "\nBut got\n";
  for (auto y = 0; y < height; ++y) {
    for (auto x = 0; x < width; ++x) {
      const auto value = actual[x + width * y];
      ss << (value == CG::INFTY ? "X " : std::to_string(value))
         << (0 <= value && value < 10 ? "  " : " ");
    }
    ss << '\n';
  }
  return ss.str();
}


TEST_CASE( "Bfs gives the correct distance field", "[bfs]" ) {
  Grid grid;
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
      CG::bfs(grid, 3, distance_field);
      std::vector<int> expected{
        3, 2, 1, 0,
        4, 3, 2, 1,
        5, 4, 3, 2,
      };
      INFO( error_msg_distance_fields(4, 3, expected, distance_field) );
      REQUIRE_THAT( distance_field, Equals(expected) );
    }

    SECTION( "From a center tile" ) {
      CG::bfs(grid, 5, distance_field);
      std::vector<int> expected{
        2, 1, 2, 3,
        1, 0, 1, 2,
        2, 1, 2, 3,
      };
      INFO( error_msg_distance_fields(4, 3, expected, distance_field) );
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

    struct TileBlockedPredicate {
      const Grid& grid;
      TileBlockedPredicate(const Grid& grid): grid{grid} {}
      bool operator()(Index index, int /* distance */) const { return grid.at(index) == Tile::B; }
    } is_blocked{grid};

    const int B = CG::INFTY;

    SECTION( "From an unblocked tile" ) {
      CG::bfs(grid, 32, distance_field, is_blocked);
      std::vector<int> expected = {
        B, B, 7, 6, B, 8,
        8, 7, 6, 5, B, 7,
        7, 6, 5, 4, 5, 6,
        8, B, 4, 3, 4, 5,
        9, B, B, 2, 3, 4,
        10,B, 0, 1, 2, 3,
        11,B, 1, 2, 3, 4,
      };
      INFO( error_msg_distance_fields(6, 7, expected, distance_field) );
      REQUIRE_THAT( distance_field, Equals(expected) );
    }

    SECTION( "From a blocked tile" ) {
      CG::bfs(grid, 4, distance_field, is_blocked);
      std::vector<int> expected = {
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1,
      };
      INFO( error_msg_distance_fields(6, 7, expected, distance_field) );
      REQUIRE_THAT( distance_field, Equals(expected) );
    }

    SECTION( "From multiple sources" ) {
      std::vector<size_t> sources{ 6, 22 };
      CG::bfs(grid, sources.begin(), sources.end(), distance_field, is_blocked);
      std::vector<int> expected = {
        B, B, 3, 4, B, 4,
        0, 1, 2, 3, B, 3,
        1, 2, 3, 2, 1, 2,
        2, B, 2, 1, 0, 1,
        3, B, B, 2, 1, 2,
        4, B, 4, 3, 2, 3,
        5, B, 5, 4, 3, 4,
      };
      INFO( error_msg_distance_fields(6, 7, expected, distance_field) );
      REQUIRE_THAT( distance_field, Equals(expected) );
    }
  }
}
