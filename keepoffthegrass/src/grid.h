#ifndef GRID_H_
#define GRID_H_

#include <vector>

#include "worldinfo.h"

/**
 * Return the tiles which are next to valid tiles not owned by us.
 */
const std::vector<
  std::vector<std::reference_wrapper<const Tile>>::size_type>&
compute_boundary(const WorldInfo& worldinfo);


#endif // GRID_H_
