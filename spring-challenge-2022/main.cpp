#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string_view>
#include <vector>

#include "types.h"

namespace spring {

auto
operator<<(std::ostream& _out, const Point& p) -> std::ostream&
{
    return _out << p.x << ' ' << p.y;
}


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

struct Action
{
    enum class Type
    {
        WAIT,
        MOVE,
        SPELL
    };
    Type type{ Type::WAIT };
    Point dest{};
    std::string_view msg{ "" };

    Action() = default;

    Action(Point dest_, std::string_view msg_ = "")
            : type{ Type::MOVE }
            , dest{ dest_ }
            , msg{ msg_ }
    {}

    explicit Action(std::string_view msg_)
            : msg{ msg_ }
    {}
};

class Game
{
public:
    Game() = default;
    void init(std::istream& _in)
    {
        int n_heroes;
        _in >> m_us.base.x >> m_us.base.y;
        _in >> n_heroes;
        m_them.base.x = WIDTH - m_us.base.x;
        m_them.base.y = HEIGHT - m_us.base.y;
        m_us.id = Player::Id::US;
        m_them.id = Player::Id::THEM;
        m_us.health = 3;
        m_them.health = 3;
        m_n_turns = 0;
    }
    void update(std::istream& _in)
    {
        m_us.heros.clear();
        m_them.heros.clear();
        m_monsters.clear();

        int entity_count;
        int _id, _type, x, y, shield_life, is_controlled, health, vx, vy, near_base, threat_for;

        _in >> m_us.health >> m_us.mana;
        _in >> m_them.health >> m_them.mana;
        _in >> entity_count;

        for (int i = 0; i < entity_count; ++i) {
            _in >> _id >> _type >> x >> y >> shield_life >> is_controlled >> health >> vx >> vy >>
              near_base >> threat_for;
            if (_type == 0) {
                Monster& monster = m_monsters.emplace_back();
                monster.id = _id;
                monster.pos.x = x;
                monster.pos.y = y;
                monster.shield_life = shield_life;
                monster.is_controlled = (bool)is_controlled;
                monster.health = health;
                monster.vel.x = vx;
                monster.vel.y = vy;
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
                Hero& hero = (_type == 1 ? m_us.heros.emplace_back() : m_them.heros.emplace_back());
                hero.id = _id;
                hero.pos.x = x;
                hero.pos.y = y;
                hero.shield_life = shield_life;
                hero.is_controlled = (bool)is_controlled;
            }
        }
        ++m_n_turns;
    }

    [[nodiscard]] const std::vector<Monster>& monsters() const { return m_monsters; }

    [[nodiscard]] Player us() const { return m_us; }

    [[nodiscard]] Player them() const { return m_them; }

private:
    Player m_us;
    Player m_them;
    std::vector<Monster> m_monsters;
    int m_n_turns;
};

inline auto
n_turns_to_kill(const Monster& monster) noexcept -> int
{
    return std::ceil((float)monster.health / 2.0f);
}
inline auto
distance_needed_to_kill(const Monster& monster, const int n_hero = 1) noexcept -> int
{
    return n_turns_to_kill(monster) * 400;
}
/**
 * Split the map into three 'triangular' regions based at our base:
 * 0 for upper, 1 for middle and 2 for bottom.
 * Initially, each corresponding hero is in charge of its own region.
 *
 * FIXME: I tweaked the values in operator() until I got regions between 0 and 3
 * but I don't know where the bug is (this isn't what it should be...)
 */
struct get_region
{
    const Point corner;

    get_region(const Point& base)
            : corner{ base.x < WIDTH / 10.0 ? Point{ 0, 0 } : Point{ WIDTH - 1, HEIGHT - 1 } }
    {
        std::cerr << "Init region getter with corner = " << corner << std::endl;
    }

    auto operator()(const Point& p) -> int
    {
        Direction dir = (p - corner).dir;
        double a = Direction::normalized(dir.angle, corner.x == 0 ? -M_PI : 0.0);
        //double corner_angle = corner.x == 0 ? a + (0.5 * M_PI) : a - (0.5 * M_PI);  // Rotate by -90 or +90 degrees to land in [0, pi/2]
        //double a = corner.x == 0 ? 6.0 * dir.angle * M_1_PI : 6.0 * dir.angle * M_1_PI + 6;
        double corner_fraction = 6.0 * M_1_PI * a + (corner.x == 0 ? 3 : -3);
        std::cerr << "Threat in region " << corner_fraction << std::endl;
        return std::floor(corner_fraction);
    }
};

class Agent
{
    struct ExtMonster
    {
        int region;
        int base_distance2;
        Monster monster;

        ExtMonster(int reg_, int bd2_, const Monster& m)
                : region{ reg_ }
                , base_distance2{ bd2_ }
                , monster{ m }
        {}
    };

    struct ExtHero
    {
        int region;
        Hero hero;
        std::vector<ExtMonster> monsters;
    };

public:
    Agent(const Game& game)
            : m_game{ game }
    {
        init();
    }

    void choose_actions(std::vector<Action>& actions)
    {
        get_threats(Player::Id::US);
        assign_monsters();

        actions.clear();

        for (int i = 0; i < 3; ++i) {
            if (m_our_heros[i].monsters.empty()) {
                actions.emplace_back("No threat in my region");
            } else {
                const Monster& monster = m_our_heros[i].monsters.front().monster;
                actions.emplace_back(target_monster(monster), "Targetting most dangerous monster");
            }
        }
    }

private:
    const Game& m_game;
    std::vector<Monster> m_our_threats;
    std::array<ExtHero, 3> m_our_heros;
    std::vector<Monster> m_their_threats;

    get_region our_regions{ m_game.us().base };

    void init()
    {
        for (int i = 0; i < 3; ++i) {
            m_our_heros[i] = ExtHero{ i, m_game.us().heros[i], {} };
        }
    }

    void get_threats(Player::Id player)
    {
        m_our_threats.clear();

        // Collect monsters which are a threat to us
        auto threat_out =
          player == Player::Id::US ? std::back_inserter(m_our_threats) : std::back_inserter(m_their_threats);
        std::copy_if(m_game.monsters().begin(),
                     m_game.monsters().end(),
                     threat_out,
                     [&player](const Monster& monster) { return monster.threat_for == player; });
    }

    void assign_monsters()
    {

        // Clear earlier assignments
        for (int i = 0; i < 3; ++i) {
            m_our_heros[i].monsters.clear();
        }

        // Assign monsters to heros per angle region
        for (const auto& m : m_our_threats) {
            int region = our_regions(m.pos);
            int base_distance2 = distance2(m.pos, m_game.us().base);
            m_our_heros[region].monsters.emplace_back(region, base_distance2, m);
        }

        // Sort each hero's monsters by distance to base
        for (int i = 0; i < 3; ++i) {
            std::sort(
              m_our_heros[i].monsters.begin(),
              m_our_heros[i].monsters.end(),
              [](const auto& em1, const auto& em2) { return em1.base_distance2 < em2.base_distance2; });
        }
    }

    auto target_monster(const Monster& monster) -> Point { return monster.pos + monster.vel; }
};

} // namespace spring

using namespace spring;

auto
operator<<(std::ostream& _out, const Action& action) -> std::ostream&
{
    _out << (action.type == Action::Type::WAIT ? "WAIT" : "MOVE");
    if (action.type == Action::Type::MOVE) {
        _out << ' ' << action.dest;
    }
    if (action.msg != "") {
        _out << ' ' << action.msg;
    }
    return _out;
}

void
output_actions(std::ostream& _out, const std::vector<Action>& actions)
{
    for (int i = 0; i < 3; ++i) {
        _out << actions[i] << std::endl;
    }
}

auto
main() -> int
{
    using std::cerr;
    using std::cin;
    using std::cout;
    using std::endl;

    Game game;
    game.init(cin);
    game.update(cin);

    Agent agent(game);

    std::vector<Action> actions;

    while (true) {
        agent.choose_actions(actions);
        output_actions(cout, actions);
        game.update(cin);
    }

    return 0;
}
