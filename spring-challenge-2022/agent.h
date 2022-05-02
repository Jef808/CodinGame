#ifndef AGENT_H_
#define AGENT_H_

#include "types.h"
#include "game.h"

#include <algorithm>
#include <vector>

namespace spring {


struct MonsterData
{
    /* The monster descriptor */
    Monster monster;
    /* Record the closest hero */
    unsigned int closest_hero{3};
    /* Record the distance to closest hero */
    double dist_closest_hero{std::numeric_limits<double>::max()};
    /* Record distance to base */
    double dist_base{std::numeric_limits<double>::max()};
    /* Number of turns until it does damage, or -1 if never */
    int n_turns_until_damage{-1};
};

class ExtMonsters {
public:
    /**
     * Return a pointer to the MonsterData of monster with id
     */
    [[nodiscard]] auto view_data(unsigned int monster_id) const -> const MonsterData* {
        return &m_data[m_indices[monster_id]];
    }

    /**
     * Pointer to monster that is closest to our base
     */
    [[nodiscard]] auto closest_to_base() const -> const MonsterData* {
        return &m_data[0];
    }

    /**
     * Pointer to monster that is closest to hero with id
     */
    [[nodiscard]] auto closest_to_hero(unsigned int hero_id) const -> const MonsterData* {
        // The comparison functor
        static auto cmp_closest = [id=hero_id](const auto& a, const auto& b) {
            if (a.closest_hero == id) {
                if (b.closest_hero == id) {
                    return a.dist_closest_hero < b.dist_closest_hero;
                }
                else {
                    return true;
                }
            }
            else {
                return false;
            }
        };

        auto it = std::min_element(m_data.begin(), m_data.end(), cmp_closest);

        return it == m_data.end() ? nullptr : &*it;
    }

    /**
     * Sort the data with respect to the Cmp binary functor
     */
    template<typename Cmp>
    void sort(Cmp cmp) {
        std::sort(m_data.begin(), m_data.end(), cmp);
        m_indices.resize(m_data.size());
        std::transform(m_data.begin(), m_data.end(), m_indices.begin(),
                       [](const auto& m) { return m.monster.id; });
    }

    /**
     * Clear the data
     */
    void clear() {
        m_data.clear();
    }

    /**
     * Push an empty MonsterData and return a reference to it
     */
    auto make_data() -> MonsterData& {
        return m_data.emplace_back();
    }

    /**
     * Check if there is any data available
     */
    [[nodiscard]] auto is_empty() const -> bool {
        return m_data.empty();
    }

private:
    /* Store the index of monsters in @m_data by id */
    std::vector<unsigned int> m_indices;
    /* The monster's data */
    std::vector<MonsterData> m_data;
};

/**
 * Helper struct to create actions for output
 *
 * TODO: - incorporate into an ExtHeros class which represents the three heros as a unified data structure.
 * - generalize the API so that the agent's commands generate optimized (triples of) action sequences
 * - e.g. heros as arrangement of circles and straight segments containing necessary updated data
 * - Further incorporate into ExtEntities data structure managing all entities as a Delaunay triangulation
 * - Adjust behavior for a certain command in terms of the current state (constraints to minimize important fog of war,
 *   concentrate heros before WIND push if needed, consider "all-in" WIND push when faced with big cluster of monster
 *   and have lots of mana, etc...)
 */
struct ActionFactory {

    auto make_wait(std::string_view msg_ = "") -> Action
    {
        return Action{ Point_None, msg_ };
    }
    auto make_move(const Point& dir, std::string_view msg_ = "") -> Action
    {
        return Action{ dir, msg_ };
    }
    auto make_wind(const Point& dir, std::string_view msg_ = "") -> Action
    {
        return Action{ Spell::Type::WIND, dir, -1, msg_ };
    }
    auto make_control(int target_id, const Point& dir, std::string_view msg_ = "") -> Action
    {
        return Action{ Spell::Type::CONTROL, dir, target_id, msg_ };
    }
    auto make_shield(int target_id, std::string_view msg_ = "") -> Action
    {
        return Action{ Spell::Type::SHIELD, target_id, msg_ };
    }
};

class Agent
{
public:
    Agent(Game& game)
            : m_game{ game }
    {
        init();
    }

    void choose_actions(std::vector<Action>& actions)
    {
        actions.clear();
        mana_used = 0;

        update();
        mana_used = 0;

        std::cerr << "Updated Agent's data" << std::endl;

        for (auto i : {1, 2, 0}) {
            if (m_threats.is_empty()) {
                actions.push_back(do_default(m_game.us().heros[i]));
            }
            else if (m_game.us().heros[i].id == 0) {
                actions.push_back(do_push_or_move_closest(m_game.us().heros[i]));
            } else {
                actions.push_back(do_defend_first(m_game.us().heros[i]));
            }
        }
    }

private:
    Game& m_game;
    const Point m_origin { 0, 0 };

    const double defa = M_PIf64 / 8.0;
    const double defb = M_PIf64 / 3.0;

    double def_radius{ 6000.0 };
    double def3_rad{ 1.4 * 19796.0 / 2.0 };
    std::array<Point, 3> default_targets;
    ExtMonsters m_threats;

    int mana_used { 0 };
    ActionFactory af;

    /**
     * Move hero towards its default target
     */
    auto do_default(const Hero& hero) -> Action {
        return af.make_move(default_targets[hero.id], "No threats! --> default");
    }

    /**
     * Move towards the target closest to our base
     */
    auto do_defend_first(const Hero& hero) -> Action {
        const MonsterData* closest_md = m_threats.closest_to_base();
        if (m_game.us().mana - mana_used >= 10 && closest_md->dist_closest_hero < 1280.0) {
            mana_used += 10;
            return af.make_wind(hero.pos + closest_md->monster.pos, "Pushing threat out of base radius");
        }
        return af.make_move(target_monster(hero, m_threats.closest_to_base()->monster), "Defending first threat");
    }

    /**
     * Push closest target towards opponent's base or move towards it
     */
    auto do_push_or_move_closest(const Hero& hero) -> Action {

        const MonsterData* closest_md = m_threats.closest_to_hero(hero.id);

        if (closest_md == nullptr) {
            return af.make_move(default_targets[hero.id], "Closest threat is closer to another hero, moving to default");
        }

        if (closest_md->dist_closest_hero < 1280.0) {
            mana_used += 10;
            return af.make_wind((hero.pos + closest_md->monster.pos) + -m_origin, "Pushing towards opponent");
        }
        else if (distance(closest_md->monster.pos, default_targets[0]) < 1280.0 + 1600.0) {
            Point target = closest_md->monster.pos + closest_md->monster.vel;
            return af.make_move(target, "Catching target to push onto opponent");
        }
        else {
            return af.make_move(default_targets[hero.id], "Closest threat is too far");
        }
    }

    auto get_region(const Point& p) -> unsigned int
    {
        double a = std::atan2(p.y, p.x);
        return a > 2 * defa ? 2 : 1;
    }

    void init()
    {
        const int def1x = std::ceil(def_radius * std::cos(defa));
        const int def1y = std::ceil(def_radius * std::sin(defa));

        const int def2x = std::ceil(def_radius * std::cos(defb));
        const int def2y = std::ceil(def_radius * std::sin(defb));

        const int def3x = std::ceil(def3_rad * 0.8906474877235223);
        const int def3y = std::ceil(def3_rad * 0.45469446072255826);

        default_targets[1] = { def1x, def1y };
        default_targets[2] = { def2x, def2y };
        default_targets[0] = { def3x, def3y };
    }

    void update()
    {
        m_threats.clear();

        // Compute the distance data
        for (const auto& m : m_game.monsters()) {
            m_threats.make_data() = get_data(m);
        }

        // Comparison functor for sorting w.r.t. distance to base
        static auto cmp_dist_base = [](const auto& a, const auto& b) { return a.dist_base < b.dist_base; };

        // Sort by distance to base
        m_threats.sort(cmp_dist_base);
    }

    [[nodiscard]] auto get_data(const Monster& m) const -> MonsterData
    {
        MonsterData data;

        data.monster = m;

        data.dist_base = distance(m_origin, m.pos);
        std::array<double, 3> d2_mhs = { distance(m.pos, m_game.us().heros[0].pos),
                                         distance(m.pos, m_game.us().heros[1].pos),
                                         distance(m.pos, m_game.us().heros[2].pos) };
        for (int i=0; i<3; ++i) {
            if (d2_mhs[i] < data.dist_closest_hero) {
                data.closest_hero = i;
                data.dist_closest_hero = d2_mhs[i];
            }
        }

        return data;
    }

    // Aim at some monster
    auto target_monster(const Hero& h, const Monster& m) -> Point {
        return m.pos + m.vel + (h.id == 1 ? Point{780, 0} : Point{0, 780});
    }
};

} // namespace spring

#endif // AGENT_H_
