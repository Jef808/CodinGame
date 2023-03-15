#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>

#include "gridutils.h"
#include "towerdef.h"

constexpr int GUNTOWER_COST = 100;
constexpr int GLUETOWER_COST = 70;

using namespace TowerDefense;

inline bool is_on_my_side(const Point& p, const Game& game) {
    bool is_on_left_side = (p.x < static_cast<int>(game.width() / 2)
                            || p.y > static_cast<int>(game.height() / 2));
    return (is_on_left_side && game.player_id() == Player::LEFT)
           || (!is_on_left_side && game.player_id() == Player::RIGHT);
}

inline std::vector<Player> sides(const Game& game) {
    std::vector<Player> ret;

    for (auto y = 0; y < game.height(); ++y) {
        for (auto x = 0; x < game.width(); ++x) {
            const bool is_on_left_side =
                    (x < static_cast<int>(std::floor(game.width() / 2))
                     || (x == static_cast<int>(std::floor(game.width() / 2)) && y > static_cast<int>(std::floor(game.height() / 2))));
            ret.push_back(is_on_left_side ? Player::LEFT : Player::RIGHT);
        }
    }

    return ret;
}

int main(int argc, char* argv[]) {

    Game game = initialize(std::cin);

    auto heat = heatmap(game);

    std::vector<size_t> indices;
    indices.resize(game.grid().tiles.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Double the heat value for tiles on our side
    std::transform(indices.begin(), indices.end(), heat.begin(), heat.begin(),
                   [id=game.player_id(), s=sides(game)](auto idx, auto h){
                     return s[idx] == id ? 2 * h : h;
                   });

    // Sort indices in decreasing order of their heat.
    std::sort(indices.begin(), indices.end(),
              [&heat](auto a, auto b) { return heat[a] > heat[b]; });

    auto it = indices.begin();

    int turn = 0;
    bool bought_glue = false;

    std::vector<std::string> actions;

    while (true) {

      std::cin >> game;

      std::cerr << "Turn " << turn << std::endl;

      actions.emplace_back("PASS");

      auto money = game.my_money();

      std::cerr << "Money: " << money << std::endl;

      while (it != indices.end()) {
        bool acted = false;

        const unsigned int x = *it % game.width();
        const unsigned int y = *it / game.width();

        // if tile is unoccupied
        const bool occupied = (game.at(x, y) & Grid::Tile::Tower);
        if (occupied) {
          ++it;
          continue;
        }

        if (!bought_glue && turn > 0 && money >= GLUETOWER_COST) {
          std::stringstream ss;
          ss << "BUILD " << x << ' ' << y << " GLUETOWER";
          actions.push_back(ss.str());

          money -= GLUETOWER_COST;
          bought_glue = true;
          acted = true;
        }
        else if (money >= GUNTOWER_COST) {
          std::stringstream ss;
          if (heat[*it] > 8.0) {
            ss << "BUILD " << x << ' ' << y << " FIRETOWER";
          }
          else {
            ss << "BUILD " << x << ' ' << y << " GUNTOWER";
          }

          actions.push_back(ss.str());

          money -= GUNTOWER_COST;
          acted = true;
        }

        if (acted) {
          ++it;
        }
        else {
          break;
        }
      }

      std::copy(actions.begin(), actions.end(), std::ostream_iterator<std::string>{std::cout, ";"});
      std::cout << "MSG hello";
      std::cout << std::endl;

      actions.clear();
      ++turn;
    }
}
