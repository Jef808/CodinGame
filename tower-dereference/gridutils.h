#ifndef GRIDUTILS_H_
#define GRIDUTILS_H_

#include <cmath>

#include "towerdef.h"

namespace TowerDefense {

/**
 * \brief The Euclidean distance between two points.
 */
inline double distance(const Point& a, const Point& b) {
  return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

/**
 * \brief To each point, assign a weight indicating its importance
 * relative to the minimum flow of attackers around it.
 *
 * More precisely, the heat of a tile at \f$p = (x, y)\f$ is given by
 * \f$h(p) = -1\f$ if \f$p\f$ is a Canyon tile, otherwise
 * \f[
 *   h(p) =
 *     \begin{cases}
 *       -1 \quad &\text{if $p$ is a \emph{Canyon} tile}, \\
 *       \sum_{p' \in B(p, 6)} \frac{w(p')}{d(p, p')} \quad &\text{if $p$ is a \emph{Plateau} tile.}
 *     \end{cases}
 * \f]
 * where the sum is taken over tiles within the disk of radius \f$6\f$ around \f$p\f$
 * and \f$w(p')\f$ is the \em weight of a tile, given by one over the
 * number of shortest paths in the canyon passing through that tile.
 */
std::vector<double> heatmap(const Game& game);

} // namespace TowerDefense

#endif // GRIDUTILS_H_
