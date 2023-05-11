#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

#include "../agent.h"

#include <vector>

using namespace Catch::Matchers;
using namespace CG;
using namespace kog;

namespace {

TEST_CASE("Compute battlefronts correctly", "[voronoi_helper]") {
    SECTION("When only two unit tiles are opposed at distance 1") {
        Game::Grid grid{5, 6};
        std::vector<Tile> tiles = {
            {.x=0,.y=0}, {.x=1,.y=0}, {.x=2,.y=0}, {.x=3,.y=0}, {.x=4,.y=0},
            {.x=0,.y=1}, {.x=1,.y=1}, {.x=2,.y=1}, {.x=3,.y=1}, {.x=4,.y=1},
            {.x=0,.y=2}, {.x=1,.y=2}, {.x=2,.y=2}, {.x=3,.y=2}, {.x=4,.y=2},
            {.x=0,.y=3}, {.x=1,.y=3,.owner=1,.units=1}, {.x=2,.y=3}, {.x=3,.y=3,.owner=0,.units=1}, {.x=4,.y=3},
            {.x=0,.y=4}, {.x=1,.y=4}, {.x=2,.y=4}, {.x=3,.y=4}, {.x=4,.y=4},
            {.x=0,.y=5}, {.x=1,.y=5}, {.x=2,.y=5}, {.x=3,.y=5}, {.x=4,.y=5},
        };
        grid.set_tiles(std::move(tiles));
        Game game{std::move(grid)};
        Agent agent{game};

        const std::vector<int> my_distance_field = {
            4, 3, 4, 5, 6,
            3, 2, 3, 4, 5,
            2, 1, 2, 3, 4,
            1, 0, 1, 2, 3,
            2, 1, 2, 3, 4,
            3, 2, 3, 4, 5
        };
        const std::vector<int> opp_distance_field = {
            6, 5, 4, 3, 4,
            5, 4, 3, 2, 3,
            4, 3, 2, 1, 2,
            3, 2, 1, 0, 1,
            4, 3, 2, 1, 2,
            5, 4, 3, 2, 3
        };
        const auto expected_my_projected_tiles = 12;
        const auto expected_opp_projected_tiles = 12;
        const std::vector<Game::Grid::index_type> expected_neutral_frontier = {
            2, 7, 12, 17, 22, 27
        };
        const std::vector<Game::Grid::index_type> expected_my_frontier = {
            1, 6, 11, 16, 21, 26
        };
        const std::vector<Game::Grid::index_type> expected_opp_frontier = {
            3, 8, 13, 18, 23, 28
        };

        agent.compute_turn_info();
        BattlefrontsInfo battlefronts_info;
        // compute_battlefronts_info(grid,
        //                           my_distance_field,
        //                           opp_distance_field,
        //                           bfi);

        // CHECK(bfi.my_projected_tiles == expected_my_projected_tiles);
        // CHECK(bfi.opp_projected_tiles == expected_opp_projected_tiles);

        // CHECK_THAT(bfi.neutral_frontier, Equals(expected_neutral_frontier));
        CHECK_THAT(battlefronts_info.my_frontier, Equals(expected_my_frontier));
        CHECK_THAT(battlefronts_info.opp_frontier, Equals(expected_opp_frontier));
    }

    SECTION("When only two unit tiles are opposed at distance 0") {
        Game::Grid grid{5, 6};
        std::vector<Tile> tiles = {
            {.x=0,.y=0}, {.x=1,.y=0}, {.x=2,.y=0}, {.x=3,.y=0}, {.x=4,.y=0},
            {.x=0,.y=1}, {.x=1,.y=1}, {.x=2,.y=1}, {.x=3,.y=1}, {.x=4,.y=1},
            {.x=0,.y=2}, {.x=1,.y=2}, {.x=2,.y=2}, {.x=3,.y=2}, {.x=4,.y=2},
            {.x=0,.y=3}, {.x=1,.y=3,.owner=1,.units=1}, {.x=2,.y=3,.owner=0,.units=1},{.x=3,.y=3}, {.x=4,.y=3},
            {.x=0,.y=4}, {.x=1,.y=4}, {.x=2,.y=4}, {.x=3,.y=4}, {.x=4,.y=4},
            {.x=0,.y=5}, {.x=1,.y=5}, {.x=2,.y=5}, {.x=3,.y=5}, {.x=4,.y=5},
        };
        Game game{std::move(grid)};
        Agent agent{game};

        grid.set_tiles(std::move(tiles));
        const std::vector<int> my_distance_field = {
            4, 3, 4, 5, 6,
            3, 2, 3, 4, 5,
            2, 1, 2, 3, 4,
            1, 0, 1, 2, 3,
            2, 1, 2, 3, 4,
            3, 2, 3, 4, 5
        };
        const std::vector<int> opp_distance_field = {
            5, 4, 3, 4, 5,
            4, 3, 2, 3, 4,
            3, 2, 1, 2, 3,
            2, 1, 0, 1, 2,
            3, 2, 1, 2, 3,
            4, 3, 2, 3, 4
        };
        const auto expected_my_projected_tiles = 12;
        const auto expected_opp_projected_tiles = 18;
        const std::vector<Game::Grid::index_type> expected_neutral_frontier = {

        };
        const std::vector<Game::Grid::index_type> expected_my_frontier = {
            1, 6, 11, 16, 21, 26
        };
        const std::vector<Game::Grid::index_type> expected_opp_frontier = {
            2, 7, 12, 17, 22, 27
        };

        agent.compute_turn_info();

        BattlefrontsInfo battlefronts_info;
        // compute_battlefronts(grid,
        //                      my_distance_field,
        //                      opp_distance_field,
        //                      bfi);

        // CHECK(bfi.my_projected_tiles == expected_my_projected_tiles);
        // CHECK(bfi.opp_projected_tiles == expected_opp_projected_tiles);

        // CHECK_THAT(bfi.neutral_frontier, Equals(expected_neutral_frontier));
        CHECK_THAT(battlefronts_info.my_frontier, Equals(expected_my_frontier));
        CHECK_THAT(battlefronts_info.opp_frontier, Equals(expected_opp_frontier));
    }
}

} // namespace
