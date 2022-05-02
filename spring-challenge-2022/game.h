#ifndef GAME_H_
#define GAME_H_

#include "types.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>


namespace spring {

struct Hero
{
    int id;
    Point pos;
    int shield_life;
    bool is_controlled;
};

struct Player
{
    enum class Id
    {
        US,
        THEM,
        NONE
    };
    Id id{ Id::NONE };
    Point base;
    int health;
    int mana;
    std::vector<Hero> heros;
};

struct Monster
{
    int id;
    Point pos;
    int shield_life;
    bool is_controlled;
    int health;
    Point vel;
    bool near_base;
    Player::Id threat_for{ Player::Id::NONE };
};

struct Spell
{
    enum class Type
    {
        NONE,
        WIND,
        CONTROL,
        SHIELD
    };

    Type type{ Type::NONE };
    int target_id{ -1 };

    [[nodiscard]] constexpr auto range() const -> int
    {
        switch (type) {
            case Type::WIND:
                return 1280;
            case Type::CONTROL:
            case Type::SHIELD:
                return 2200;
            default:
                assert(false && "No range for Null spell");
        }
    }
};

struct Action
{
    enum class Type
    {
        WAIT,
        MOVE,
        SPELL
    };

    Type type{ Type::WAIT };
    Point target{ Point_None };
    Spell spell{};
    std::string msg;

    Action(Point target_ = Point_None, std::string_view msg_ = "")
            : type{ target_ != Point_None ? Type::MOVE : Type::WAIT }
            , target{ target_ }
            , msg{ msg_ }
    {}

    Action(Spell::Type spell_type_, Point target_, int id_ = -1, std::string_view msg_ = "")
            : type{ Type::SPELL }
            , target{ target_ }
            , spell{ spell_type_, id_ }
            , msg{ msg_ }
    {}

    Action(Spell::Type spell_type_, int id_ = -1, std::string_view msg_ = "")
            : type{ Type::SPELL }
            , spell{ spell_type_, id_ }
            , msg{ msg_ }
    {}


};

class Game
{
public:
    Game() = default;
    void init(std::istream& input)
    {
        int n_heroes;
        input >> m_us.base.x >> m_us.base.y;
        input >> n_heroes;
        m_them.base.x = WIDTH - 1 - m_us.base.x;
        m_them.base.y = HEIGHT - 1 - m_us.base.y;
        m_should_reflect = m_us.base.x > WIDTH / 2;
        m_us.id = Player::Id::US;
        m_them.id = Player::Id::THEM;
        m_us.health = 3;
        m_them.health = 3;
        m_n_turns = 0;
    }
    void update(std::istream& input)
    {
        m_us.heros.clear();
        m_them.heros.clear();
        m_monsters.clear();

        int entity_count;
        int id, type, x, y, shield_life, is_controlled, health, vx, vy, near_base, threat_for;

        input >> m_us.health >> m_us.mana;
        input >> m_them.health >> m_them.mana;
        input >> entity_count;

        for (int i = 0; i < entity_count; ++i) {
            input >> id >> type >> x >> y >> shield_life >> is_controlled >> health >> vx >> vy >>
              near_base >> threat_for;
            if (type == 0) {
                Monster& monster = m_monsters.emplace_back();
                monster.id = id;
                monster.pos.x = normalized_x(x);
                monster.pos.y = normalized_y(y);
                monster.shield_life = shield_life;
                monster.is_controlled = (bool)is_controlled;
                monster.health = health;
                monster.vel.x = normalized_x(vx);
                monster.vel.y = normalized_y(vy);
                monster.near_base = (bool)near_base;
                switch (threat_for) {
                    case 0:
                        monster.threat_for = Player::Id::NONE;
                        break;
                    case 1:
                        monster.threat_for = Player::Id::US;
                        break;
                    case 2:
                        monster.threat_for = Player::Id::THEM;
                        break;
                    default:
                        assert(false && "Unknown 'threat_for' input");
                }
            } else {
                Hero& hero = (type == 1 ? m_us.heros.emplace_back() : m_them.heros.emplace_back());
                hero.id = id;
                hero.pos.x = normalized_x(x);
                hero.pos.y = normalized_y(y);
                hero.shield_life = shield_life;
                hero.is_controlled = (bool)is_controlled;
            }
        }
        ++m_n_turns;
    }

    [[nodiscard]] auto normalized_x(int x) const -> int {
        return m_should_reflect ? x = WIDTH - x - 1 : x;
    }

    [[nodiscard]] auto normalized_y(int y) const -> int {
        return m_should_reflect ? y = HEIGHT - y - 1 : y;
    }

    auto normalize(Point& p) const -> Point& {
        p.x = normalized_x(p.x);
        p.y = normalized_y(p.y);
        return p;
    }

    [[nodiscard]] auto monsters() const -> const std::vector<Monster>& { return m_monsters; }

    [[nodiscard]] auto us() const -> Player { return m_us; }

    [[nodiscard]] auto them() const -> Player { return m_them; }

private:
    Player m_us;
    Player m_them;
    std::vector<Monster> m_monsters;
    int m_n_turns;
    bool m_should_reflect;
};

}  // namespace spring

#endif // GAME_H_
