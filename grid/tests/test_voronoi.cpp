#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

#include "grid/grid.h"
#include "grid/voronoi.h"

#include "helpers.h"

#include <sstream>
#include <vector>

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

TEST_CASE( "Voronoi diagram is generated correctly", "[voronoi]" ) {
  using Index = Grid<Tile>::index_type;

  Grid<Tile> grid;
  const int X = INT::INFTY;
  std::vector<CG::VoronoiTileDescriptor<Tile>> voronoi;

  SECTION( "With only one site" ) {
    const Index width = 6;
    const Index height = 7;

    std::vector<Tile> tiles = {
      B, B, F, F, B, F,
      F, F, F, F, B, F,
      F, F, F, F, F, F,
      F, B, F, F, F, F,
      F, B, B, F, F, F,
      F, B, F, F, F, F,
      F, B, F, F, F, F,
    };

    grid.set_dimensions(width, height);
    grid.set_tiles(std::move(tiles));

    REQUIRE(grid.width() * grid.height() == 42);

    const Index a = 6;
    std::vector<Index> sites(1, a);

    generate_voronoi_diagram(grid,
                             sites.begin(),
                             sites.end(),
                             voronoi);

    std::vector<int> expected_distance_field {
      X, X, 3, 4, X, 8,
      0, 1, 2, 3, X, 7,
      1, 2, 3, 4, 5, 6,
      2, X, 4, 5, 6, 7,
      3, X, X, 6, 7, 8,
      4, X, 8, 7, 8, 9,
      5, X, 9, 8, 9, 10
    };

    std::vector<std::vector<Index>> expected_sites {
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

    std::for_each(voronoi.begin(), voronoi.end(), [&](const auto& cell) {
      actual_distance_field.push_back(cell.distance());
      actual_sites.emplace_back(cell.sites().begin(), cell.sites().end());
    });

    INFO(error_msg_distance_fields(width, height, actual_distance_field, expected_distance_field));

    REQUIRE_THAT(actual_distance_field, Equals(expected_distance_field));
    REQUIRE_THAT(actual_sites, Equals(expected_sites));
  }

  SECTION( "With three sites" ) {
    const Index width = 6;
    const Index height = 7;

    std::vector<Tile> tiles = {
      B, B, F, F, B, F,
      F, F, F, F, B, F,
      F, F, F, F, F, F,
      F, B, F, F, F, F,
      F, B, B, F, F, F,
      F, B, F, F, F, F,
      F, B, F, F, F, F,
    };

    grid.set_dimensions(width, height);
    grid.set_tiles(std::move(tiles));

    const Index a = grid.index_of(5, 1);
    const Index b = grid.index_of(0, 2);
    const Index c = grid.index_of(3, 4);

    std::vector<Index> sites{a, b, c};

    generate_voronoi_diagram(grid, sites.begin(), sites.end(), voronoi);

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

    std::for_each(voronoi.begin(),
                  voronoi.end(),
                  [&](const auto& cell) {
                    actual_distance_field.push_back(cell.distance());
                    actual_sites.emplace_back(cell.sites().begin(), cell.sites().end());
                  });

    INFO(error_msg_distance_fields(width, height, actual_distance_field, expected_distance_field));

    REQUIRE_THAT(actual_distance_field, Equals(expected_distance_field));
    REQUIRE_THAT(actual_sites, Equals(expected_sites));
  }

  SECTION("On a small 3x3 grid") {
    std::vector<Tile> tiles{
      F, F, F,
      F, F, F,
      F, F, F
    };
    std::vector<int> units{
      0, 1, 0,
      1, 0, 1,
      0, 1, 0
    };

    grid.set_dimensions(3, 3);
    grid.set_tiles(std::move(tiles));

    std::vector<Index> sites{
      grid.index_of(1, 0),
      grid.index_of(0, 1),
      grid.index_of(2, 1),
      grid.index_of(1, 2)
    };

    REQUIRE_THAT(sites, Equals(std::vector<Index>{1, 3, 5, 7}));

    std::vector<int> expected_distance_field{
      1, 0, 1,
      0, 1, 0,
      1, 0, 1
    };

    std::vector<std::vector<Index>> expected_sites{
      {1, 3}, {1         }, {1, 5},
      {3   }, {1, 3, 5, 7}, {5   },
      {3, 7}, {7         }, {5, 7}
    };

    generate_voronoi_diagram(grid, sites.begin(), sites.end(), voronoi);

    std::vector<int> actual_distance_field;
    std::vector<std::vector<Index>> actual_sites;

    std::for_each(voronoi.begin(), voronoi.end(), [&](const auto& cell) {
      actual_distance_field.push_back(cell.distance());
      actual_sites.emplace_back(cell.sites().begin(), cell.sites().end());
    });

    REQUIRE_THAT(actual_distance_field, Equals(expected_distance_field));
    REQUIRE_THAT(actual_sites, Equals(expected_sites));
  }

  SECTION("On a larger grid with many sites") {
    std::vector<Tile> tiles{
      F, F, B, F, F, F, F, F, B, F, B, F,
      F, F, F, F, F, F, F, F, F, F, F, F,
      F, F, F, F, F, B, F, F, F, F, F, F,
      F, F, F, F, F, F, B, F, F, F, F, F,
      F, F, F, F, F, F, F, F, F, F, F, F,
      F, B, F, B, F, F, F, F, F, B, F, F
    };
    const Index a = grid.index_of(1, 0);
    const Index b = grid.index_of(0, 1);
    const Index c = grid.index_of(2, 1);
    const Index d = grid.index_of(1, 2);
    const Index w = grid.index_of(10, 3);
    const Index x = grid.index_of(9, 4);
    const Index y = grid.index_of(11, 4);
    const Index z = grid.index_of(10, 5);

    std::vector<int> expected_distance_field{
      1, 0, X, 2, 3, 4, 5, 6, X, 4, X, 4,
      0, 1, 0, 1, 2, 3, 4, 5, 4, 3, 2, 3,
      1, 0, 1, 2, 3, X, 5, 4, 3, 2, 1, 2,
      2, 1, 2, 3, 4, 5, X, 3, 2, 1, 0, 1,
      3, 2, 3, 4, 5, 6, 5, 2, 1, 0, 1, 0,
      4, X, 4, X, 6, 5, 4, 3, 2, X, 0, 1
    };

    grid.set_dimensions(12, 6);
    grid.set_tiles(std::vector<Tile>{tiles.begin(), tiles.end()});
    std::vector<Index> sites{
      grid.index_of(1, 0),
      grid.index_of(0, 1),
      grid.index_of(2, 1),
      grid.index_of(1, 2),
      grid.index_of(10, 3),
      grid.index_of(9, 4),
      grid.index_of(11, 4),
      grid.index_of(10, 5)
    };

    generate_voronoi_diagram(grid,
                             sites.begin(),
                             sites.end(),
                             voronoi);

    std::vector<int> actual_distance_field;
    std::vector<std::vector<Index>> actual_sites;

    std::for_each(voronoi.begin(), voronoi.end(), [&](const auto& cell) {
      actual_distance_field.push_back(cell.distance());
      actual_sites.emplace_back(cell.sites().begin(), cell.sites().end());
    });

    auto index = 0;
    for (const auto& cell : voronoi) {
      for (auto site : cell.sites()) {
        REQUIRE(std::find(sites.begin(), sites.end(), site) != sites.end());
      }
    }

    REQUIRE(voronoi[grid.index_of(1, 0)].sites() == std::set<Index>{
        grid.index_of(1, 0)
      });
    REQUIRE(voronoi[grid.index_of(1, 3)].sites() == std::set<Index>{
        grid.index_of(1, 2)
      });
    REQUIRE(voronoi[grid.index_of(0, 0)].sites() == std::set<Index>{
        grid.index_of(1, 0), grid.index_of(0, 1)
      });
    REQUIRE(voronoi[grid.index_of(1, 1)].sites() == std::set<Index>{
        grid.index_of(1, 0),
        grid.index_of(0, 1),
        grid.index_of(2, 1),
        grid.index_of(1, 2),
      });
  }

  SECTION( "Same grid, calling with distance field version", "[voronoi]" ) {
    const auto U = CG::INT::UNVISITED;

    const auto width = 12;
    const auto height = 6;
    std::vector<Tile> tiles{
      F, F, B, F, F, F, F, F, B, F, B, F,
      F, F, F, F, F, F, F, F, F, F, F, F,
      F, F, F, F, F, B, F, F, F, F, F, F,
      F, F, F, F, F, F, B, F, F, F, F, F,
      F, F, F, F, F, F, F, F, F, F, F, F,
      F, B, F, B, F, F, F, F, F, B, F, F
    };

    grid.set_dimensions(width, height);
    grid.set_tiles(std::move(tiles));

    std::vector<int> distance_field{
      1, 0, X, 2, 3, 4, 5, 6, X, 4, X, 4,
      0, 1, 0, 1, 2, 3, 4, 5, 4, 3, 2, 3,
      1, 0, 1, 2, 3, X, 5, 4, 3, 2, 1, 2,
      2, 1, 2, 3, 4, 5, X, 3, 2, 1, 0, 1,
      3, 2, 3, 4, 5, 6, 5, 2, 1, 0, 1, 0,
      4, X, 4, X, 6, 5, 4, 3, 2, X, 0, 1
    };

    std::vector<std::vector<Index>> voronoi_sites;
    generate_voronoi_diagram(grid, distance_field, voronoi_sites);

    REQUIRE_THAT(voronoi_sites[grid.index_of(1, 0)], UnorderedEquals(std::vector<Index>{grid.index_of(1, 0)}));

    REQUIRE_THAT(voronoi_sites[grid.index_of(1, 3)], UnorderedEquals(std::vector<Index>{grid.index_of(1, 2)}));

    REQUIRE_THAT(voronoi_sites[grid.index_of(0, 0)], UnorderedEquals(std::vector<Index>{
        grid.index_of(1, 0),
        grid.index_of(0, 1)
    }));

    REQUIRE_THAT(voronoi_sites[grid.index_of(1, 1)], UnorderedEquals(std::vector<Index>{
        grid.index_of(1, 0),
        grid.index_of(0, 1),
        grid.index_of(2, 1),
        grid.index_of(1, 2)
    }));
  }
}
