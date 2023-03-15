#ifndef TOWERDEF_H_
#define TOWERDEF_H_

#include <iosfwd>
#include <vector>

namespace TowerDefense {

constexpr int START_MONEY = 350;
constexpr int START_LIVES = 20;
constexpr int MAP_WIDTH = 17;
constexpr int MAP_HEIGHT = 17;

enum class Player { NONE, LEFT, RIGHT };

struct Point {
    int x;
    int y;
};

struct Entity : Point {
    virtual ~Entity() = default;

    int id{-1};
    Player owner{Player::NONE};
};

struct Attacker : Entity {
    int hp;
    int max_hp;
    double speed;
    double max_speed;
    double slow_time;
    int bounty;
};

struct Tower : Entity {
    enum Type { Gun, Fire, Glue, Heal };

    Type type;
    int damage;
    double range;
    int reload;
    int cooldown;
};

struct Grid {
    enum Tile {
      Tower = 1, Attacker = 2,
      Canyon = 4, Plateau = 8
    };
    std::vector<Tile> tiles;
};

class Game {
  public:
    Game() = default;

    [[nodiscard]] int my_money() const { return m_my_money; }

    [[nodiscard]] int opp_money() const { return m_opp_money; }

    [[nodiscard]] int my_lives() const { return m_my_lives; }

    [[nodiscard]] int opp_lives() const { return m_opp_lives; }

    [[nodiscard]] Player player_id() const { return m_player_id; }

    [[nodiscard]] int width() const { return m_width; }

    [[nodiscard]] int height() const { return m_height; }

    [[nodiscard]] const Grid& grid() const { return m_grid; }

    [[nodiscard]] const std::vector<Tower>& towers() const { return m_towers; }

    [[nodiscard]] const std::vector<Attacker>& attackers() const {
        return m_attackers;
    }

    [[nodiscard]] Grid::Tile at(int x, int y) const;
    [[nodiscard]] Grid::Tile at(const Point& p) const;

    friend Game initialize(std::istream& is);
    void input(std::istream& is);

  private:
    int m_my_money{START_MONEY};
    int m_opp_money{START_MONEY};
    int m_my_lives{START_LIVES};
    int m_opp_lives{START_LIVES};
    Player m_player_id{Player::NONE};
    int m_width{MAP_WIDTH};
    int m_height{MAP_HEIGHT};
    Grid m_grid;
    std::vector<Tower> m_towers;
    std::vector<Attacker> m_attackers;

};

extern Game initialize(std::istream& is);

} // namespace TowerDefense

inline std::istream& operator>>(std::istream& is, TowerDefense::Game& game) {
    game.input(is);
    return is;
}

#endif // TOWERDEF_H_
