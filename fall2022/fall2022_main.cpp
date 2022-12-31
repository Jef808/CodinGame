#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

static constexpr int ME = 1;
static constexpr int OPP = 0;
static constexpr int NONE = -1;

struct Point {
  private:
    static inline int game_width{0};
    static inline int game_height{0};
    static inline bool dimension_set{false};

  public:
    int x;
    int y;

    static void set_dimensions(int w, int h) {
        if (!dimension_set) {
            game_width = w;
            game_height = h;
            dimension_set = true;
        } else {
            throw std::runtime_error("Point dimension is already set.");
        }
    }

    [[nodiscard]] std::vector<Point> construct_neighbours() const {
        if (!dimension_set)
            throw std::runtime_error("Point: dimension not set!");

        std::vector<Point> neighbours;
        const std::array<Point, 4> candidates{
                Point{    x, y - 1},
                Point{x - 1,                      y},
                Point{x + 1,                      y},
                Point{    x, y + 1}
        };
        std::copy_if(candidates.begin(), candidates.end(),
                     std::back_inserter(neighbours), [&](const Point& p){ return in_bounds(p); });
        return neighbours;
    }

    [[nodiscard]] bool in_bounds(const Point& p) const {
        return p.x >= 0 && p.y >= 0 && p.x < game_width
               && p.y < game_height;
    }

    operator size_t() const { return x + y * game_width; }

    ostream& dump(ostream& ioOut) const { return ioOut << x << " " << y; }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator<(const Point& other) const {
        return y < other.y || (y == other.y && x < other.x);
    }

    Point operator+(const Point& other) const {
        return {x + other.x, y + other.y};
    }
};

inline std::vector<Point> construct_neighbours(const Point& point) {
    assert(point.x >= 0 && point.y >= 0 && "point not initialized"); // NOLINT
    return point.construct_neighbours();
}

struct Tile {
    int x{-1}, y{-1};
    int scrap_amount{0}, owner{0}, units{0};
    bool recycler{false}, can_build{false}, can_spawn{false},
            in_range_of_recycler{false};
    std::vector<Point> neighbours;

    Tile() = delete;

    Tile(int x, int y)
        : x{x}
        , y{y}
        , neighbours{construct_neighbours(*this)} { }

    operator Point() const { return Point{x, y}; }

    operator size_t() const { return (size_t)(Point) * this; }

    [[nodiscard]] std::vector<Point>::const_iterator nbh_begin() const {
        return neighbours.begin();
    }

    [[nodiscard]] std::vector<Point>::const_iterator nbh_end() const {
        return neighbours.end();
    }

    [[nodiscard]] bool walkable() const {
        return scrap_amount > 0 && !recycler;
    }
};

ostream& operator<<(ostream& ioOut, const Tile& tile) {
    return ((Point)tile).dump(ioOut);
}

struct Game {
    int width;
    int height;
    int my_matter{0};
    int opp_matter{0};

    vector<Tile> tiles;

    vector<const Tile*> my_tiles;
    vector<const Tile*> opp_tiles;
    vector<const Tile*> neutral_tiles;
    vector<const Tile*> my_units;
    vector<const Tile*> opp_units;
    vector<const Tile*> my_recyclers;
    vector<const Tile*> opp_recyclers;

    Game(int width, int height)
        : width{width}
        , height{height} {
        Point::set_dimensions(width, height);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                tiles.emplace_back(x, y);
            }
        }
    }

    template <typename OutputIter>
    void neighbours(const Tile& tile, OutputIter itOut) const {
        std::transform(tile.nbh_begin(), tile.nbh_end(), itOut,
                       [&](const auto& idx) { return &tiles[idx]; });
    }

    void reset_info() {
        for (auto* vec : {&my_tiles, &opp_tiles, &neutral_tiles, &my_units,
                          &opp_units, &my_recyclers, &opp_recyclers})
            vec->clear();
    }

    ostream& dump(ostream& ioOut) const {
        for (const auto& tile : tiles) {
            ioOut << tile << " ";
        }
        return ioOut;
    }

    void update_turn(istream& ioIn) {
        reset_info();

        ioIn >> my_matter >> opp_matter;
        ioIn.ignore();
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int scrap_amount = 0;
                int owner = 0; // 1 = me, 0 = foe, -1 = neutral
                int units = 0;
                int recycler = 0;
                int can_build = 0;
                int can_spawn = 0;
                int in_range_of_recycler = 0;
                ioIn >> scrap_amount >> owner >> units >> recycler >> can_build
                        >> can_spawn >> in_range_of_recycler;
                ioIn.ignore();

                Tile& tile = tiles[x + y * width];
                tile.scrap_amount = scrap_amount;
                tile.owner = owner;
                tile.units = units;
                tile.recycler = recycler == 1;
                tile.can_build = can_build == 1;
                tile.can_spawn = can_spawn == 1;
                tile.in_range_of_recycler = in_range_of_recycler == 1;

                if (tile.owner == ME) {
                    my_tiles.push_back(&tile);
                    if (tile.units > 0) {
                        my_units.push_back(&tile);
                    } else if (tile.recycler) {
                        my_recyclers.push_back(&tile);
                    }
                } else if (tile.owner == OPP) {
                    opp_tiles.push_back(&tile);
                    if (tile.units > 0) {
                        opp_units.push_back(&tile);
                    } else if (tile.recycler) {
                        opp_recyclers.push_back(&tile);
                    }
                } else {
                    neutral_tiles.push_back(&tile);
                }
            }
        }
    }
};

inline int nb_expanses(int my_matter) {
    static constexpr double COST = 10.0;
    return static_cast<int>(std::floor(my_matter / COST));
}

std::string make_spawn(int amount, const Tile& tile) {
    ostringstream action;
    action << "SPAWN " << amount << " " << tile;
    return action.str();
}

std::string make_move(int amount, const Tile& source, const Tile& target) {
    ostringstream action;
    action << "MOVE " << amount << " " << source << " " << target;
    return action.str();
}

std::string make_build(const Tile& tile) {
    ostringstream action;
    action << "BUILD " << tile;
    return action.str();
}

class BFS {
  private:
    enum class Color { WHITE, GRAY, BLACK };

  public:
    explicit BFS(Game& game)
        : game{game}
  {
    dirty.resize(size, true);
    distances.resize(size);
  }

    void set_all_dirty() { std::fill_n(dirty.begin(), size, true); }

    /**
     * Note: this only resets the `distances` array for given tile,
     * but it does reset the whole color and parent arrays
     */
    void initialize_for(const Tile& tile) {
        source_ = &tile;

        color.resize(size);
        std::fill_n(color.begin(), size, Color::WHITE);

        distances[tile].resize(size);
        std::fill_n(distances[tile].begin(), size,
                    std::numeric_limits<double>::max());

        parent.resize(size);
        std::transform(vertices.begin(), vertices.end(), parent.begin(),
                       pointer_traits<const Tile*>::pointer_to);

        queue.clear();
        queue.push_back(&tile);

        color[tile] = Color::GRAY;
        distances[tile][tile] = 0.0;
    }

    /**
     * Compute the length of the shortest path from each tile to `source`.
     */
    void compute_distances_from(const Tile& source) {

        initialize_for(source);
        std::vector<double>& distance = distances[source];

        while (!queue.empty()) {
            const Tile* cur = queue.front();
            queue.pop_front();

            buffer.clear();
            game.neighbours(*cur, std::back_inserter(buffer));

            for (const Tile* nex : buffer) {

                if (!nex->walkable()) {
                    continue;
                }

                // If *nex has not yet been discovered and is not blocked
                if (color[*nex] == Color::WHITE) {
                    color[*nex] = Color::GRAY;
                    distance[*nex] = distance[*cur] + 1.0;
                    parent[*nex] = cur;
                    queue.push_back(nex);
                }

                // If *nex has been discovered but not examined,
                // check if this provides a shortest path
                // Note: No need to update all the distances between
                // source and nex in this case, since those vertices
                // will not be part of the shortest path...
                else if (color[*nex] == Color::GRAY) {
                    if (distance[*nex] > distance[*cur] + 1.0) {
                        distance[*nex] = distance[*cur] + 1.0;
                        parent[*nex] = cur;
                    }
                }
                // If *nex has been fully examined, we move on
            }
            // Mark current vertex as black after examining it
            color[*cur] = Color::BLACK;
        }

        dirty[source] = false;
    }

    const std::vector<double>& distance_function(const Tile& tile) {
        if (dirty[tile]) compute_distances_from(tile);
        return distances[tile];
    }

  private:
    const Game& game;
    const std::vector<Tile>& vertices{game.tiles};
    const size_t size{vertices.size()};

    const Tile* source_{nullptr};
    std::deque<const Tile*> queue;
    std::vector<Color> color;
    std::vector<const Tile*> parent;
    std::vector<std::vector<double>> distances;
    std::vector<const Tile*> buffer;
    std::vector<bool> dirty;
};

struct Agent {
    explicit Agent(Game& game)
        : game{game}
        , bfs{game} { }

    /**
     * Return the sum of shortest distances over `sources` tiles for each
     * `targets` tiles.
     */
    template <typename TileContainer, typename OtherTileContainer>
    std::vector<std::pair<const Tile*, double>>
            avg_target_distance(const TileContainer& sources,
                              const OtherTileContainer& targets) {

        const auto avg_distance = [&](double total_targets_units) {
            return [&, total = total_targets_units](const Tile* const& source) {
                return std::accumulate(
                        targets.begin(), targets.end(), 0.0,
                        [&dist_fn = bfs.distance_function(*source), &total](
                                double sum, const Tile* const& target) mutable {
                            return sum
                                   + dist_fn[*target] * (target->units / total);
                        });
            };
        };

        const double total_target_units =
                std::accumulate(targets.begin(), targets.end(), 0.0,
                                [](double sum, const Tile* const& target) {
                                    return sum + target->units;
                                });

        std::vector<std::pair<const Tile*, double>> result;
        std::transform(
                sources.begin(), sources.end(), std::back_inserter(result),
                [&, avg_distance = avg_distance(total_target_units)](
                        const Tile* const& source) {
                    return std::make_pair(source, avg_distance(source));
                });

        return result;
    }

    /**
     * Return the closest target for each source
     */
    template <typename TileContainer, typename OtherTileContainer>
    std::vector<std::pair<const Tile*, const Tile*>>
            closest_target(const TileContainer& sources,
                           const OtherTileContainer& targets) {
        const auto closest = [&](const Tile* const& source) {
            return *std::min_element(
                    targets.begin(), targets.end(),
                    [&dist_fn = bfs.distance_function(*source)](
                            const Tile* const& target_a,
                            const Tile* const& target_b) {
                        return dist_fn[*target_a] < dist_fn[*target_b];
                    });
        };

        std::vector<std::pair<const Tile*, const Tile*>> result;

        std::transform(sources.begin(), sources.end(),
                       std::back_inserter(result),
                       [&closest](const Tile* const& source) {
                           return std::make_pair(source, closest(source));
                       });

        return result;
    }

    const std::vector<string>& get_actions() {
        buffer.clear();
        spawn_actions.clear();
        build_actions.clear();
        move_actions.clear();

        bfs.set_all_dirty();
        bdry_dirty = true;

        compute_boundary();

        int nb_resource = nb_expanses(game.my_matter);

        // SPAWNS
        if (nb_resource > 0) {
            // Find the point on the boundary with the smallest average
            // distance to enemies and spawn all there
            auto avg_distance =
                    avg_target_distance(my_inner_bdry, game.opp_units);
            const Tile* hottest_bdry_tile =
                    std::min_element(avg_distance.begin(), avg_distance.end(),
                                     [](const auto& a, const auto& b) {
                                         return a.second < b.second;
                                     })
                            ->first;

            spawn_actions.emplace_back(
                    std::make_tuple(nb_resource, hottest_bdry_tile));
        }

        // MOVES
        {
            auto min_distance = closest_target(game.my_units, my_outer_bdry);
            for (const auto& [source, target] : min_distance) {
                move_actions.emplace_back(
                        std::make_tuple(source->units, source, target));
            }
        }

        return stringify_actions();
    }

  private:
    Game& game;
    BFS bfs;
    bool bdry_dirty{true};
    std::vector<const Tile*> buffer;
    std::set<const Tile*> my_inner_bdry;
    std::set<const Tile*> my_outer_bdry;
    std::vector<std::tuple<int, const Tile*>> spawn_actions;
    std::vector<const Tile*> build_actions;
    std::vector<std::tuple<int, const Tile*, const Tile*>> move_actions;
    std::vector<string> actions;

    const std::vector<std::string>& stringify_actions() {
        actions.clear();

        std::transform(build_actions.begin(), build_actions.end(),
                       std::back_inserter(actions),
                       [](const auto& action) { return make_build(*action); });
        std::transform(spawn_actions.begin(), spawn_actions.end(),
                       std::back_inserter(actions), [](const auto& action) {
                           return make_spawn(get<0>(action), *get<1>(action));
                       });
        std::transform(move_actions.begin(), move_actions.end(),
                       std::back_inserter(actions), [](const auto& action) {
                           return make_move(get<0>(action), *get<1>(action),
                                            *get<2>(action));
                       });

        return actions;
    }

    // Look for tiles we own which are next to a tile we don't own.
    void compute_boundary() {

        if (!bdry_dirty) {
            return;
        }
        my_inner_bdry.clear();
        my_outer_bdry.clear();

        for (const auto* tile : game.my_tiles) {
            buffer.clear();
            game.neighbours(*tile, std::back_inserter(buffer));

            for (const auto* nbh : buffer) {
                if (nbh->owner != ME && nbh->walkable()) {
                    my_inner_bdry.insert(tile);
                    my_outer_bdry.insert(nbh);
                }
            }
        }

        bdry_dirty = false;
    }
};

int main() {
    try {
        int width = 0, height = 0;
        cin >> width >> height;
        cin.ignore();

        Game game{width, height};
        Agent agent{game};

        // game loop
        while (true) {
            game.update_turn(cin);

            const auto& actions = agent.get_actions();

            if (actions.empty()) {
                cout << "WAIT" << endl;
            } else {

              std::copy(actions.begin(), actions.end(),
                        std::ostream_iterator<std::string>{cout, ";"});
              cout << endl;
            }
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
