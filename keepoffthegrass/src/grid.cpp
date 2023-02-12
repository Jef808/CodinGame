#include "grid.h"

#include <algorithm>
#include <set>

using namespace std;
using size_type = std::vector<std::reference_wrapper<const Tile>>::size_type;

const std::vector<size_type>& compute_boundary(const WorldInfo& worldinfo) {
    static std::vector<size_type> result;
    result.clear();

    const auto tile = [&](unsigned long idx) {
        return worldinfo.world().tile(idx);
    };
    const auto is_bdry = [&](unsigned long nbh) {
        return tile(nbh).owner != ME && tile(nbh).scrap_amount > 0
               && !tile(nbh).recycler;
    };
    std::set<size_type> uniq;

    for (const Tile& my_tile : worldinfo.my_tiles()) {
        const auto index = worldinfo.world().get_idx(my_tile.x, my_tile.y);

        if (!tile(index).can_spawn) {
            continue;
        }
        const auto width = worldinfo.world().width();
        const auto height = worldinfo.world().height();

        if (index + 1 % width != 0 && is_bdry(index + 1)) {
            uniq.insert(index + 1);
        }
        if (index + width < width * height && is_bdry(index + width)) {
            uniq.insert(index + width);
        }
        if (index > 0 && index - 1 % width != width - 1 && is_bdry(index - 1)) {
            uniq.insert(index - 1);
        }
        if (index - width >= 0 && is_bdry(index - width)) {
            uniq.insert(index - width);
        }
    }
    std::copy(uniq.begin(), uniq.end(), std::back_inserter(result));
    return result;
}
