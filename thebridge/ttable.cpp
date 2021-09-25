#include "ttable.h"

#include <cmath>
#include <cstring>
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

void TranspositionTable::resize(size_t power_of_two) {
    size_t new_cluster_count = (size_t(1) << power_of_two);

    if (new_cluster_count == cluster_count) return;
    cluster_count = new_cluster_count;

    free(mem);
    mem = calloc(1, cluster_count * sizeof(Cluster) + CacheLineSize - 1);

    if (!mem)
    {
        std::cerr << "Failed to allocate " << cluster_count
                  << "clusters for transposition table" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cerr << "Allocated "
                  << cluster_count * sizeof(Cluster) / 1024 / 1024
            << " mb for transposition table" << std::endl;
    }

    table = (Cluster*)((uintptr_t(mem) + CacheLineSize - 1) & ~(CacheLineSize - 1));
}

// Overwrites the TranspositionTable with zeros.
void TranspositionTable::clear() {
    std::memset(table, 0, cluster_count * sizeof(Cluster));
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
