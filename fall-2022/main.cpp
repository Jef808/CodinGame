#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

enum class Player { None, Me, Them };

enum class PredicateName { CanBuild, CanSpawn, InRangeOfRecycler };

struct Tile {
    Player owner{Player::None};
    int amount{0};
    int units{0};
    bool recycler{false};
};

struct TileProps {
    bool can_build{false};
    bool can_spawn{false};
    bool in_range_of_recycler{false};
};

class Game {
  public:
    Game() = default;

    friend Game init_game(std::istream& in);
    friend void update_game(Game& game, std::istream& in);
    friend class Agent;
    friend void view(const Game& game, std::ostream& out);

  private:
    std::vector<Tile> m_tiles;
    std::vector<TileProps> m_tiles_props;
    int m_width{0};
    int m_height{0};
    int m_matter_me{0};
    int m_matter_them{0};
};

Game init_game(std::istream& in) {
    Game game;
    in >> game.m_width >> game.m_height;
    in.ignore();
    const auto nb_tiles = game.m_width * game.m_height;
    game.m_tiles.resize(nb_tiles, {});
    game.m_tiles_props.resize(nb_tiles, {});
    return game;
}

void update_game(Game& game, std::istream& in) {
    in >> game.m_matter_me >> game.m_matter_them;
    in.ignore();
    for (int i = 0; i < game.m_width * game.m_height; ++i) {
        Tile& tile = game.m_tiles[i];
        TileProps& props = game.m_tiles_props[i];
        int owner = 0, recycler = 0;
        int can_build = 0, can_spawn = 0, in_range_of_recycler = 0;
        in >> tile.amount >> owner >> tile.units >> tile.recycler >> can_build
                >> can_spawn >> in_range_of_recycler;
        in.ignore();

        tile.owner = owner == 1   ? Player::Me
                     : owner == 0 ? Player::Them
                                  : Player::None;
        tile.recycler = recycler == 1;

        props.can_build = can_build == 1;
        props.can_spawn = can_spawn == 1;
        props.in_range_of_recycler = in_range_of_recycler == 1;
    }
}

void view(const Game& game, std::ostream& out) {
    out << "width: " << game.m_width << ", height: " << game.m_height
        << ", my matter: " << game.m_matter_me
        << ", their matter: " << game.m_matter_them << std::endl;
    out << "Buildable tiles:\n";
    for (auto i = 0; i < game.m_width * game.m_height; ++i) {
        if (game.m_tiles_props[i].can_build) {
            out << '(' << i % game.m_width << ", " << i / game.m_width << "); ";
        }
    }
    out << std::endl;
}

class SelectRandom {
  public:
    SelectRandom() = default;

    void operator()(size_t nb_choices, size_t nb_picks,
                    std::vector<size_t>& picks, bool repeatable = false) {

        buf.resize(nb_choices, 0);
        std::iota(buf.begin(), buf.end(), 0);

        if (not repeatable) {
            assert(nb_choices >= nb_picks // NOLINT
                   && "Failed assertion: nb_choices >= nb_picks");

            if (nb_choices == nb_picks) {
                std::copy(buf.begin(), buf.end(), std::back_inserter(picks));
                return;
            }
            for (size_t i = 0; i < nb_picks; ++i) {
                size_t next = std::uniform_int_distribution<size_t>{
                        i, nb_choices - 1}(gen);
                std::swap(buf[i], buf[next]);
            }
            std::copy_n(buf.begin(), nb_picks, std::back_inserter(picks));
        } else {
            auto dist =
                    std::uniform_int_distribution<size_t>{0, nb_choices - 1};
            for (size_t i = 0; i < nb_picks; ++i)
                picks.push_back(dist(gen));
        }
    }

    bool flip_fair_coin() { return std::uniform_int_distribution<>{0, 2}(gen); }

    template <typename I> I rand_int(I min, I max) {
        if (min > max) std::swap(min, max);
        return std::uniform_int_distribution<I>(min, max)(gen);
    }

    auto rand_int_chooser(int min, int max) {
        if (min > max) {
            std::swap(min, max);
        }
        return [&, dist = std::uniform_int_distribution<>(min, max)]() mutable {
            return dist(gen);
        };
    }

  private:
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::vector<int> buf;
    std::vector<std::uniform_int_distribution<>> dists;
};

class Agent {
    static constexpr double COST = 10.0;

  public:
    Agent() = default;

    void build_randomly(const Game& game, std::vector<std::string>& actions);
    void spawn_randomly(const Game& game, std::vector<std::string>& actions);
    void move_randomly(const Game& game, std::vector<std::string>& actions);

    void update_data(const Game& game);

  private:
    SelectRandom selector{};
    int my_matter{0};
    std::vector<size_t> my_build_actions;
    std::vector<size_t> my_tiles;
    std::vector<size_t> opp_tiles;
    std::vector<size_t> neutral_tiles;
    std::vector<size_t> my_units;
    std::vector<size_t> opp_units;
    std::vector<size_t> my_recyclers;
    std::vector<size_t> opp_recyclers;
};

void Agent::update_data(const Game& game) {
    my_tiles.clear();
    opp_tiles.clear();
    neutral_tiles.clear();
    my_units.clear();
    opp_units.clear();
    my_recyclers.clear();
    opp_recyclers.clear();

    for (auto i = 0; i < game.m_width * game.m_height; ++i) {
        auto owner = game.m_tiles[i].owner;
        switch (owner) {
        case Player::Me: {
            my_tiles.push_back(i);
            if (game.m_tiles[i].units > 0) my_units.push_back(i);
            else if (game.m_tiles[i].recycler)
                my_recyclers.push_back(i);
            break;
        }
        case Player::Them: {
            opp_tiles.push_back(i);
            if (game.m_tiles[i].units > 0) opp_units.push_back(i);
            else if (game.m_tiles[i].recycler)
                opp_recyclers.push_back(i);
            break;
        }
        case Player::None:
            neutral_tiles.push_back(i);
            break;
        }
    }
}

void Agent::build_randomly(const Game& game,
                           std::vector<std::string>& actions) {

    static std::vector<size_t> buildable;
    static std::vector<size_t> build_picks;
    buildable.clear();
    build_picks.clear();

    for (size_t i = 0; i < game.m_tiles.size(); ++i) {
        if (game.m_tiles_props[i].can_build) buildable.push_back(i);
    }

    auto nb_build_max =
            static_cast<size_t>(std::floor((game.m_matter_me) / COST));
    std::cerr << "Can build " << std::min(nb_build_max, buildable.size())
              << " out of the " << buildable.size() << " buildable tiles ";
    for (auto i : buildable)
        std::cerr << '(' << i % game.m_width << ", " << i / game.m_width
                  << "); ";
    std::cerr << std::endl;

    nb_build_max = std::min(nb_build_max, buildable.size());

    selector(buildable.size(), nb_build_max, build_picks);

    for (auto i : build_picks) {
        size_t idx = my_build_actions.emplace_back(buildable[i]);
        my_matter -= COST;
        const int x = static_cast<int>(idx) % game.m_width;
        const int y = static_cast<int>(idx) / game.m_width;
        std::ostringstream ss;
        ss << "BUILD " << x << ' ' << y;
        actions.emplace_back(ss.str());
    }
}

void Agent::spawn_randomly(const Game& game,
                           std::vector<std::string>& actions) {
    static constexpr double COST = 10.0;

    static std::vector<size_t> spawnable;
    static std::vector<size_t> amounts;
    static std::vector<size_t> spawn_picks;
    spawnable.clear();
    spawn_picks.clear();

    for (size_t i = 0; i < game.m_tiles.size(); ++i) {
        if (game.m_tiles_props[i].can_spawn
            && std::count(my_build_actions.begin(), my_build_actions.end(), i)
                       == 0)
            spawnable.push_back(i);
    }

    auto nb_spawn_max =
            static_cast<size_t>(std::floor((game.m_matter_me) / COST));

    std::cerr << "Can spawn " << (spawnable.size() == 0 ? 0 : nb_spawn_max)
              << " on the " << spawnable.size() << " spawnable tiles ";
    for (auto i : spawnable)
        std::cerr << '(' << i % game.m_width << ", " << i / game.m_width
                  << "); ";
    std::cerr << std::endl;

    nb_spawn_max = spawnable.size() == 0 ? 0 : nb_spawn_max;

    while (true) {
        size_t nb_spawn = amounts.emplace_back(
                selector.rand_int<size_t>(1, nb_spawn_max));
        int next_nb_spawn =
                static_cast<int>(nb_spawn_max) - static_cast<int>(nb_spawn);
        if (next_nb_spawn < 0) {
            nb_spawn = nb_spawn_max;
        }
        spawn_picks.push_back(
                spawnable[selector.rand_int<size_t>(0, spawnable.size() - 1)]);

        if (next_nb_spawn <= 0) break;
    }

    for (size_t i = 0; i < spawn_picks.size(); ++i) {
        auto spawn_idx = spawn_picks[i];
        const size_t x = spawn_idx % game.m_width;
        const size_t y = spawn_idx / game.m_width;
        const size_t amount = amounts[i];
        std::ostringstream ss;
        ss << "SPAWN " << amount << ' ' << x << ' ' << y;
        actions.emplace_back(ss.str());
    }
}

void Agent::move_randomly(const Game& game, std::vector<std::string>& actions) {
    static std::vector<size_t> amounts;
    static std::vector<size_t> to_idx;
    const auto nb_tiles = game.m_width * game.m_height;

    for (auto i : my_units) {
        amounts.clear();
        to_idx.clear();
        size_t nb_units = game.m_tiles[i].units;
        while (true) {
            size_t nb_move = amounts.emplace_back(
                    selector.rand_int<size_t>(1, nb_units));
            int next_nb_units =
                    static_cast<int>(nb_units) - static_cast<int>(nb_move);
            if (next_nb_units < 1) break;
            nb_units = static_cast<size_t>(next_nb_units);
        }

        selector(nb_tiles, amounts.size(), to_idx, true);

        const size_t from_x = i % game.m_width;
        const size_t from_y = i / game.m_width;
        for (size_t amt : amounts) {
            if (selector.flip_fair_coin()) {
                const auto to_idx = selector.rand_int<size_t>(0, nb_tiles - 1);
                const size_t to_x = to_idx % game.m_width;
                const size_t to_y = to_idx / game.m_width;
                std::ostringstream oss;
                oss << "MOVE " << amt << ' ' << from_x << ' ' << from_y << ' '
                    << to_x << ' ' << to_y;
                actions.emplace_back(oss.str());
            }
        }
    }
}

int main(int argc, char* argv[]) {

    Game game = init_game(std::cin);

    std::vector<std::string> actions;
    Agent agent;

    while (true) {
        update_game(game, std::cin);
        view(game, std::cerr);

        actions.clear();
        agent.update_data(game);
        agent.build_randomly(game, actions);
        agent.spawn_randomly(game, actions);
        agent.move_randomly(game, actions);

        if (actions.empty()) actions.emplace_back("WAIT");

        for (const auto& action : actions) {
            std::cout << action << ';';
        }
        std::cout << std::endl;
    }

    return 0;
}
