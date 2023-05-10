#ifndef VORONOI_H_
#define VORONOI_H_

#include "grid.h"
#include "constants.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <set>

namespace CG {

template <typename Tile>
class VoronoiTileDescriptor {
public:
  using Index = typename Grid<Tile>::index_type;

  VoronoiTileDescriptor() = default;

  void add_site(Index index) { m_sites.insert(index); }
  void set_distance(int distance) { m_distance = distance; }
  void clear() {
    m_distance = INT::INFTY;
    m_sites.clear();
  }

  [[nodiscard]] int distance() const { return m_distance; }
  [[nodiscard]] const std::set<Index>& sites() const { return m_sites; }

private:
  int m_distance{INT::INFTY};
  std::set<Index> m_sites;
};

template <typename Tile,
          typename IndexIterator>
void generate_voronoi_diagram(
    const Grid<Tile>& grid,
    IndexIterator sites_beg,
    IndexIterator sites_end,
    std::vector<VoronoiTileDescriptor<Tile>>& voronoi_out) {
  using Grid = Grid<Tile>;
  using Index = typename Grid::index_type;

  voronoi_out.resize(grid.width() * grid.height());
  std::for_each(voronoi_out.begin(),
                voronoi_out.end(),
                [](auto& cell){ cell.clear(); });

  std::queue<Index> queue;

  std::for_each(sites_beg, sites_end, [&](auto site) {
    if (!grid.at(site).is_blocked()) {
      voronoi_out[site].set_distance(0);
      voronoi_out[site].add_site(site);
      queue.push(site);
    }
  });

  while (!queue.empty()) {
    const auto& current_index = queue.front();
    queue.pop();
    const auto current_distance = voronoi_out[current_index].distance();
    const auto& current_sites = voronoi_out[current_index].sites();

    for (const auto& neighbour_index: grid.neighbours_of(current_index)) {
      auto tentative_distance = current_distance + 1;
      if (grid.at(neighbour_index).is_blocked(tentative_distance)) {
        continue;
      }
      if (tentative_distance <= voronoi_out[neighbour_index].distance()) {
        std::for_each(current_sites.begin(), current_sites.end(), [&](auto site) {
          voronoi_out[neighbour_index].add_site(site);
        });
      }
      if (tentative_distance < voronoi_out[neighbour_index].distance()) {
        voronoi_out[neighbour_index].set_distance(tentative_distance);
        queue.push(neighbour_index);
      }
    }
  }
}

namespace {

template <typename DistanceT, typename IndexT>
inline void initialize_generate_voronoi_diagram(const std::vector<DistanceT>& distance_field,
                                                std::queue<IndexT>& queue,
                                                std::vector<std::vector<IndexT>>& voronoi_sites) {
  voronoi_sites.clear();
  std::transform(distance_field.begin(), distance_field.end(), std::back_inserter(voronoi_sites),
                 [&queue, n=0](auto distance) mutable {
                   std::vector<IndexT> sites{};
                   if (distance == 0) {
                     sites.push_back(n);
                     queue.push(n);
                   }
                   ++n;
                   return sites;
                 });
}

} // namespace

// Compute the point-to-sites map from a distance field.
template <typename GridT, typename IndexT = typename GridT::index_type>
void generate_voronoi_diagram(const GridT& grid,
                              const std::vector<int>& distance_field,
                              std::vector<std::vector<IndexT>>& voronoi_sites) {
  std::queue<IndexT> queue;
  std::set<IndexT> seen;
  initialize_generate_voronoi_diagram(distance_field, queue, voronoi_sites);

  while (!queue.empty()) {
    auto current_index = queue.front();
    queue.pop();
    const auto& current_distance = distance_field[current_index];
    const auto& current_sites = voronoi_sites[current_index];

    for (const auto nbh_index : grid.neighbours_of(current_index)) {
      const auto& nbh_distance = distance_field[nbh_index];
      if (nbh_distance == current_distance + 1) {
        auto& nbh_sites = voronoi_sites[nbh_index];
        std::copy(current_sites.begin(), current_sites.end(), std::back_inserter(nbh_sites));
        auto [_, is_first_time_seen] = seen.insert(nbh_index);
        if (is_first_time_seen) {
          queue.push(nbh_index);
        }
      }
    }
  }
}

template <typename Tile>
void generate_voronoi_diagram(const Grid<Tile>& grid,
                              std::vector<VoronoiTileDescriptor<Tile>>& voronoi) {
  using Grid = Grid<Tile>;
  using Index = typename Grid::index_type;

  std::queue<Index> queue;

  // Read the sites from the provided voronoi diagram
  std::for_each(voronoi.begin(), voronoi.end(), [&](const auto& cell) {
    if (!cell.sites().empty()) {
      queue.push(*cell.sites().begin());
    }
  });

  while (!queue.empty()) {
    const auto& current_index = queue.front();
    queue.pop();
    const auto current_distance = voronoi[current_index].distance();
    const auto& current_sites = voronoi[current_index].sites();

    for (const auto& neighbour_index: grid.neighbours_of(current_index)) {
      auto tentative_distance = current_distance + 1;
      if (grid.at(neighbour_index).is_blocked(tentative_distance)) {
        continue;
      }
      if (tentative_distance <= voronoi[neighbour_index].distance()) {
        std::for_each(current_sites.begin(), current_sites.end(), [&](auto site) {
          voronoi[neighbour_index].add_site(site);
        });
      }
      if (tentative_distance < voronoi[neighbour_index].distance()) {
        voronoi[neighbour_index].set_distance(tentative_distance);
        queue.push(neighbour_index);
      }
    }
  }
}


} // namespace CG

#endif // VORONOI_H_
