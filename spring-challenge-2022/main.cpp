#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <vector>

template<typename E,
         typename U=std::conditional_t<
             std::is_enum_v<E>,
             std::underlying_type_t<E>,
             std::conditional_t<
                 std::is_integral_v<E>,
                 E,
                 void>
             >
         >
constexpr auto to_integral(E e) -> U { return static_cast< U >(e); }

constexpr int WIDTH = 0x44DF;
constexpr int HEIGHT = 0x2329;
constexpr int MAX_MOVE = 800;
constexpr int MAX_TURNS = 220;

inline constexpr auto encode(int x, int y) -> uint32_t {
    return ((unsigned)(y & 0xFFFF) << 16) | (x & 0xFFFF);
}
enum class Offset : uint32_t { NONE = encode(0, 0),
                               EAST = encode(1, 0),  WEST = encode(-1, 0),
                               SOUTH = encode(0, 1), NORTH = encode(0, -1),
                               SOUTH_EAST = SOUTH | EAST, SOUTH_WEST = SOUTH | WEST, NORTH_EAST = NORTH | EAST, NORTH_WEST = NORTH | WEST
};
enum class Corner : uint32_t { NORTH_WEST = encode(0, 0),
                               NORTH_EAST = encode(WIDTH-1, 0),
                               SOUTH_EAST = encode(WIDTH-1, HEIGHT-1),
                               SOUTH_WEST = encode(0, HEIGHT-1)
};
inline auto constexpr get_x(Offset off) -> int { return static_cast<int16_t>(to_integral(off) & 0xFFFF); }
inline auto constexpr get_y(Offset off) -> int { return static_cast<int16_t>(to_integral(off) >> 16); }
inline auto constexpr Vertical(Offset off) -> Offset { return Offset(encode(0, get_y(off))); }
inline auto constexpr Horizontal(Offset off) -> Offset { return Offset(encode(get_x(off), 0)); }

struct Direction {
    double angle{ 0.0 };

    constexpr Direction() = default;

    explicit constexpr Direction(double angle_, double normalization=-M_PI)
        : angle { normalized(angle_, normalization) }
    {}

    /** Normalize the angle between (theta, theta + 2*PI] */
    static constexpr auto normalized(double angle_, const double theta) -> double {
        return angle_ + (angle_ < theta + 0.000001 ? -1 : 1) * std::floor(angle_ / (theta + 2*M_PI));
    }

    static constexpr auto offset(const Direction& dir) -> Offset {
        constexpr std::array<Offset, 8> offsets_thresholds { Offset::WEST, Offset::SOUTH_WEST, Offset::SOUTH, Offset::SOUTH_EAST, Offset::EAST, Offset::NORTH_EAST, Offset::NORTH, Offset::NORTH_WEST };
        const double a = normalized(dir.angle, -M_PI);
        for (int i = 0; i < 8; ++i) {
            if (a < -M_PI + (2*i + 1) * M_PI / 8) {
                return offsets_thresholds[i];
            }
        }
        return Offset::WEST;
    }
};

struct Point
{
    int x;
    int y;

    constexpr Point() : x{ WIDTH }, y{ HEIGHT - 1 }
    {}
    constexpr Point(int x_, int y_) : x{ x_ }, y{ y_ }
    {}
    explicit constexpr Point(Offset off) : x { get_x(off) }, y { get_y(off) }
    {}

    constexpr auto operator!=(const Point& other) const noexcept -> bool { return this->x != other.x || this->y != other.y; }
    constexpr auto operator==(const Point& other) const noexcept -> bool { return this->x == other.x && this->y == other.y; }

    [[nodiscard]] inline auto constexpr direction_of(const Point& p, double normalization=-M_PI) const -> Direction {
        auto dx = static_cast<double>(p.x - this->x);
        auto dy = static_cast<double>(p.y - this->y);
        return Direction{ std::atan2(dy, dx) };
    }
    [[nodiscard]] constexpr auto encoded(const Point& p) const noexcept -> uint32_t { return encode(x, y); }
};

inline auto constexpr operator+(const Point& a, const Point& b) -> Point { return { a.x + b.x, a.y + b.y }; }
inline auto constexpr operator+(const Point& p, Offset off) -> Point { return p + Point{off}; }

inline auto constexpr directed_floor(double x_, double y_, Direction dir) -> Point {
    const auto off = Direction::offset(dir);
    Point p{};

    if (Horizontal(off) == Offset::EAST) { p.x = std::floor(x_); }
    else { p.x = std::ceil(x_); }

    if (Vertical(off) == Offset::SOUTH) { p.y = std::floor(y_); }
    else { p.y = std::ceil(y_); }

    return p;
}

inline auto constexpr dot(const Point& a, const Point& b) -> double { return a.x * b.x + a.y * b.y; }
inline auto constexpr distance2(const Point& a, const Point& b) -> int { return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y); }
inline auto constexpr distance(const Point& a, const Point& b) -> double { return std::sqrt(distance2(a, b)); }
auto operator<<(std::ostream& _out, const Point& p) -> std::ostream& { return _out << p.x << ' ' << p.y; }

struct Vector {
    Point p;
    Direction dir;
    double norm;
};
inline auto constexpr operator*(const Vector& v, double s) -> Vector {
    return { v.p, v.dir, v.norm * s };
}
inline auto constexpr operator-(const Point& a, const Point& b) -> Vector {
    return { b, b.direction_of(a), distance(a, b) };
}
inline constexpr auto Flow(const Vector& v) -> Vector {
    double x = v.p.x + v.norm * std::cos(v.dir.angle);
    double y = v.p.y + v.norm * std::sin(v.dir.angle);
    return { directed_floor(x, y, v.dir), v.dir, v.norm };
}

constexpr auto Point_None = Point{};
constexpr inline auto Vector_Zero(const Point& p) -> Vector { return { p, Direction{}, 0.0 }; }


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
        MOVE
    };
    Type type { Type::WAIT };
    Point dest { };
    std::string_view msg { "" };

    Action() = default;

    Action(Point dest_, std::string_view msg_ = "")
            : type{ Type::MOVE }
            , dest{ dest_ }
            , msg{ msg_ }
    {}

    explicit Action(std::string_view msg_)
        : msg{msg_}
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
struct
get_region {
    const Point corner;

    get_region(const Point& base) : corner{ base.x < WIDTH / 10.0 ? Point{ 0, 0 } : Point{ WIDTH-1, HEIGHT-1 } }
    {
        std::cerr << "Init region getter with corner = " << corner << std::endl;
    }

    auto operator()(const Point& p) -> int {
        Direction dir = (p - corner).dir;
        double a = corner.x == 0 ? 6.0 * dir.angle * M_1_PI : 6.0 * dir.angle * M_1_PI + 6;
        std::cerr << "Threat in region " << a << std::endl;
        return std::floor(a);
    }
};

class Agent
{
    struct ExtMonster {
        int region;
        int base_distance2;
        Monster monster;

        ExtMonster(int reg_, int bd2_, const Monster& m)
            : region{ reg_ }
            , base_distance2{ bd2_ }
            , monster{ m }
        {}
    };

    struct ExtHero {
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
            }
            else {
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

    void init() {
        for (int i=0; i<3; ++i) {
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

    void assign_monsters() {

        // Clear earlier assignments
        for (int i=0; i<3; ++i) {
            m_our_heros[i].monsters.clear();
        }

        // Assign monsters to heros per angle region
        for (const auto& m : m_our_threats) {
            int region = our_regions(m.pos);
            int base_distance2 = distance2(m.pos, m_game.us().base);
            m_our_heros[region].monsters.emplace_back(region, base_distance2, m);
        }

        // Sort each hero's monsters by distance to base
        for (int i=0; i<3; ++i) {
            std::sort(m_our_heros[i].monsters.begin(), m_our_heros[i].monsters.end(), [](const auto& em1, const auto& em2) {
                return em1.base_distance2 < em2.base_distance2;
            });
        }
    }

    auto target_monster(const Monster& monster) -> Point {
        return monster.pos + monster.vel;
    }
};

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
