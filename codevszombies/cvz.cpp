#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>


const std::array<unsigned long long, 101> Fibonacci = []{
    std::array<unsigned long long, 101> ret {1, 1,};
    for (auto it = ret.begin() + 2; it != ret.end(); ++it)
        *it = *(it - 1) + *(it - 2);
    return ret;
}();

struct Entity {
    int id{-1};
    int x{-1};
    int y{-1};
    int xnext{-1};
    int ynext{-1};
    int target_id{-1};
    int distance_to_target{std::numeric_limits<int>::max()};
    int distance_to_player{-1};
};

inline double distance(const Entity& a, const Entity& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

int main() {
    std::vector<Entity> humans;
    std::vector<Entity> zombies;

    while (1) {
        int xtarget = -1;
        int ytarget = -1;

        humans.clear();
        zombies.clear();

        Entity player{};
        std::cin >> player.x >> player.y; std::cin.ignore();

        int human_count;
        std::cin >> human_count; std::cin.ignore();
        for (int i = 0; i < human_count; i++) {
            Entity& human = humans.emplace_back();
            std::cin >> human.id >> human.x >> human.y; std::cin.ignore();

            human.distance_to_player = distance(human, player);
        }

        int zombie_count;
        std::cin >> zombie_count; std::cin.ignore();
        for (int i = 0; i < zombie_count; i++) {
            // Input
            Entity& zombie = zombies.emplace_back();
            std::cin >> zombie.id >> zombie.x >> zombie.y >> zombie.xnext >> zombie.ynext; std::cin.ignore();

            zombie.distance_to_player = distance(zombie, player);

            // Identify target and record its distance
            zombie.distance_to_target = zombie.distance_to_player;
            for (const auto& human : humans) {
                double dist = distance(zombie, human);
                if (dist < zombie.distance_to_target) {
                    zombie.target_id = human.id;
                    zombie.distance_to_target = dist;
                }
            }
        }

        std::sort(zombies.begin(), zombies.end(), [](const auto& a, const auto& b) {
            return a.distance_to_target < b.distance_to_target;
        });

        for (const auto& zombie : zombies) {
            if (zombie.target_id == -1) {
                continue;
            }

            const auto& target = *std::find_if(humans.begin(), humans.end(), [id=zombie.target_id](const auto& human) {
                return human.id == id;
            });

            Entity player_target;
            player_target.x = zombie.xnext;
            player_target.y = zombie.ynext;

            int nturns = std::ceil(zombie.distance_to_target - 400 / 400);

            for (int turn = 0; turn < nturns + 1; ++turn) {
                double dist_player_target = distance(player, player_target);

                double player_dx = (player_target.x - player.x) / dist_player_target;
                double player_dy = (player_target.y - player.y) / dist_player_target;

                Entity after_travel;
                after_travel.x = player.x + 1000 * turn * player_dx;
                after_travel.y = player.y + 1000 * turn * player_dy;
                double distance_after_travel = distance(player_target, after_travel);
                bool target_reached_after_travel = distance_after_travel < 2000;

                if (target_reached_after_travel) {
                    xtarget = player_target.x;
                    ytarget = player_target.y;
                    break;
                }

                double next_distance = distance(target, player_target);
                player_target.x = std::floor(player_target.x + 400 * (target.x - player_target.x) / next_distance);
                player_target.y = std::floor(player_target.y + 400 * (target.y - player_target.y) / next_distance);
            }

            if (xtarget != -1) {
                break;
            }
        }

        if (xtarget < 0) {
            std::sort(zombies.begin(), zombies.end(), [](const auto& a, const auto& b) {
                return a.distance_to_player < b.distance_to_player;
            });
            xtarget = zombies.front().xnext;
            ytarget = zombies.front().ynext;
        }

        std::cout << xtarget << ' ' << ytarget << std::endl;
    }
}
