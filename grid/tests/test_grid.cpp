#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

#include "grid/grid.h"
#include "grid/bfs.h"
#include "grid/voronoi.h"

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
  bool operator!=(const Tile& other) const { return type != other.type; }
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
      std::vector<size_t> sources{ 6, 22 };
      bfs(grid, sources.begin(), sources.end(), distance_field);
      std::vector<int> expected = {
        X, X, 3, 4, X, 4,
        0, 1, 2, 3, X, 3,
        1, 2, 3, 2, 1, 2,
        2, X, 2, 1, 0, 1,
        3, X, X, 2, 1, 2,
        4, X, 4, 3, 2, 3,
        5, X, 5, 4, 3, 4,
      };
      INFO(error_msg_distance_fields(6, 7, distance_field, expected));
      REQUIRE_THAT( distance_field, Equals(expected) );
    }
  }
}

TEST_CASE( "Voronoi diagram is generated correctly", "[voronoi]" ) {
  using Index = Grid<Tile>::index_type;

  const Index width = 6;
  const Index height = 7;
  Grid<Tile> grid{width, height};

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

  const int X = INT::INFTY;
  std::vector<CG::VoronoiTileDescriptor<Tile>> voronoi;

  CHECK(grid.width() * grid.height() == 42);

  SECTION( "The first time, with one site" ) {
    const Grid<Tile>::index_type a = 6;
    std::vector<Grid<Tile>::index_type> sites{a};

    generate_voronoi_diagram(grid, sites.begin(), sites.end(), voronoi);

    std::vector<VoronoiTileDescriptor<Tile>> expected {
      {X, { }}, {X, { }}, {3, {a}}, {4, {a}}, {X, { }}, {4, {a}},
      {0, {a}}, {1, {a}}, {2, {a}}, {3, {a}}, {X, { }}, {3, {a}},
      {1, {a}}, {2, {a}}, {3, {a}}, {2, {a}}, {1, {a}}, {2, {a}},
      {2, {a}}, {X, { }}, {2, {a}}, {1, {a}}, {0, {a}}, {1, {a}},
      {3, {a}}, {X, { }}, {X, { }}, {2, {a}}, {1, {a}}, {2, {a}},
      {4, {a}}, {X, { }}, {4, {a}}, {3, {a}}, {2, {a}}, {3, {a}},
      {5, {a}}, {X, { }}, {5, {a}}, {4, {a}}, {3, {a}}, {4, {a}}
    };

    std::vector<int> expected_distance_field {
      X, X, 3, 4, X, 4,
      0, 1, 2, 3, X, 3,
      1, 2, 3, 2, 1, 2,
      2, X, 2, 1, 0, 1,
      3, X, X, 2, 1, 2,
      4, X, 4, 3, 2, 3,
      5, X, 5, 4, 3, 4
    };

    std::vector<std::vector<Grid<Tile>::index_type>> expected_sites {
      { }, { }, {a}, {a}, { }, {a},
      {a}, {a}, {a}, {a}, { }, {a},
      {a}, {a}, {a}, {a}, {a}, {a},
      {a}, { }, {a}, {a}, {a}, {a},
      {a}, { }, { }, {a}, {a}, {a},
      {a}, { }, {a}, {a}, {a}, {a},
      {a}, { }, {a}, {a}, {a}, {a}
    };

    std::vector<int> actual_distance_field;
    std::vector<std::vector<Index>> actual_sites;

    std::for_each(expected.begin(), expected.end(), [&](const auto& cell) {
      actual_distance_field.push_back(cell.distance);
      actual_sites.emplace_back(std::move(cell.sites));
    });

    {
      INFO(error_msg_distance_fields(width, height, actual_distance_field, expected_distance_field));
      REQUIRE_THAT(actual_distance_field, Equals(expected_distance_field));
    }

    REQUIRE_THAT(actual_sites, Equals(expected_sites));
  }

  SECTION( "The second time, with three sites" ) {
    const Index a = 10;
    const Index b = 11;
    const Index c = 26;

    std::vector<Grid<Tile>::index_type> sites{a, b, c};

    generate_voronoi_diagram(grid, sites.begin(), sites.end(), voronoi);

    std::vector<CG::VoronoiTileDescriptor<Tile>> expected{
      {X, { }}, {X, { }}, {4, {b}}, {4, {c}}, {X, { }}, {1, {a}},
      {1, {b}}, {2, {b}}, {3, {b}}, {3, {c}}, {X, { }}, {0, {a}},
      {0, {b}}, {1, {b}}, {2, {b}}, {2, {c}}, {2, {a}}, {1, {a}},
      {1, {b}}, {X, { }}, {2, {c}}, {1, {c}}, {2, {c}}, {2, {a}},
      {2, {b}}, {X, { }}, {X, { }}, {0, {c}}, {1, {c}}, {2, {c}},
      {3, {b}}, {X, { }}, {2, {c}}, {1, {c}}, {2, {c}}, {3, {c}},
      {4, {b}}, {X, { }}, {3, {c}}, {2, {c}}, {3, {c}}, {4, {c}}
    };

    std::vector<int> expected_distance_field{
      X, X, 4, 4, X, 1,
      1, 2, 3, 3, X, 0,
      0, 1, 2, 2, 2, 1,
      1, X, 2, 1, 2, 2,
      2, X, X, 0, 1, 2,
      3, X, 2, 1, 2, 3,
      4, X, 3, 2, 3, 4
    };

    std::vector<std::vector<Index>> expected_sites{
      { }, { }, {b}, {c}, { }, {a},
      {b}, {b}, {b}, {c}, { }, {a},
      {b}, {b}, {b}, {c}, {a}, {a},
      {b}, { }, {c}, {c}, {c}, {a},
      {b}, { }, { }, {c}, {c}, {c},
      {b}, { }, {c}, {c}, {c}, {c},
      {b}, { }, {c}, {c}, {c}, {c}
    };

    std::vector<int> actual_distance_field;
    std::vector<std::vector<Index>> actual_sites;

    std::for_each(expected.begin(), expected.end(), [&](const auto& cell) {
      actual_distance_field.push_back(cell.distance);
      actual_sites.emplace_back(std::move(cell.sites));
    });

    {
      INFO(error_msg_distance_fields(width, height, actual_distance_field, expected_distance_field));
      REQUIRE_THAT(actual_distance_field, Equals(expected_distance_field));
    }

    REQUIRE_THAT(actual_sites, Equals(expected_sites));
  }
}
