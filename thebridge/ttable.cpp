#include "ttable.h"
#include <iostream>


namespace tb {

TranspositionTable TT;

namespace {

/**
 * Return the number of leading 0s starting from the most
 * significant bit set.
*/
size_t msb(size_t s) {
  return __builtin_clzll(s);
}

} // namespace


/**
 * NOTE: With small state spaces as in thebridge, we can actually save every single state
 * in the transposition table for very little memory:
 *
 * #states = 500*50*16 = 400000 and cluster_size = 5. So with new_cluster_count = 80000,
 * we get enough room for every state and the required memory in mb is about 2.44mb
 */
void TranspositionTable::resize(size_t mb_size) {
    // i.e. How many bits does it take to cover the range [0, ..., mb_size * 1024 * 1024] / sizeof(Cluster) ?
    // ---> Look at log in base 2 and round up.
    size_t new_cluster_count = size_t(1) << msb((mb_size * 1024 * 1024) / sizeof(Cluster));

    if (new_cluster_count == cluster_count) return;

    cluster_count = new_cluster_count;

    free(mem);
    // NOTE: calloc zero-initializes the allocated memory.
    mem = calloc(cluster_count * sizeof(Cluster) + CacheLineSize - 1, 1);

    if (!mem)
    {
        std::cerr << "Failed to allocate " << mb_size
                  << "MB for transposition table." << std::endl;
        exit(EXIT_FAILURE);
    }

    table = (Cluster*)((uintptr_t(mem) + CacheLineSize - 1) & ~(CacheLineSize - 1));
}

TTEntry* TranspositionTable::probe(const Key key, bool& found) const {
    TTEntry* tte = first_entry(key);
    const uint16_t key16 = key >> 16;  // The key we use inside the cluster
    for (int i=0; i<cluster_size; ++i)
        if (!tte[i].key16 || tte[i].key16 == key16)  // Note: the empty entries all appear first
        {
            return found = (bool)tte[i].key16, &tte[i];
        }
    // Replace a value that's not a known_loss having worse value
    // Note: value here measures that the game is not stalling.
    TTEntry* replace = tte;
    for (int i=0; i<cluster_size; ++i)
        if ( replace->value16 > -9999 && replace->value16 > tte[i].value16 )
            replace = &tte[i];
    return found = false, replace;
}


} // namespace tb
