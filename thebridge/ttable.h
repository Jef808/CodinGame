#ifndef TTABLE_H_
#define TTABLE_H_

#include "types.h"


namespace tb {
    /// 6 bytes TT entry
    struct TTEntry {
        Action action() const { return (Action)action8; }
        Value  value()  const { return (Value)(value16); }
        size_t depth()  const { return (size_t)(depth8); }

        // Force keys to be distinct if we overwrite a nontrivial action
        void save(Key k, Value v, int d, Action a) {
            if (a != Action::None || (k >> 16) != key16) {
                action8 = (uint8_t)to_int(a);
            }
            if ( (k >> 16) != key16
                 || (v == Value::Known_win || v == Value::Known_loss))
            {
                key16 = (uint16_t)(k >> 16);
                value16 = (int16_t)(v);
                action8 = (uint8_t)(a);
                depth8 = (uint8_t)(d);
            }
        }

    private:
        friend class TranspositionTable;
        uint16_t key16;
        int16_t value16;
        uint8_t action8;
        uint8_t depth8;
    };

    /**
     * A TranspositionTable consists of a certain power of 2 of Clusters, each
     * containing a certain number of TTEntries. By making sure that the Clusters
     * have size dividing the size of our Cache, we avoid any Cache miss when querying
     * the table.
     */
    class TranspositionTable {
        static const int CacheLineSize = 64;
        static const int cluster_size = 5;

        // Clusters of 32 bytes that align with the Cache size.
        struct Cluster {
            TTEntry entry[cluster_size];
            char padding[2];
        };

        static_assert(CacheLineSize % sizeof(Cluster) == 0, "Cluster size incorrect");

    public:
        ~TranspositionTable() { free(mem); }

        void resize(size_t mb_size);
        void resize2(size_t pow2);
        void clear();

        // Return true and a pointer to the entry if found, otherwise
        // false and a pointer to an empty of least valuable TTEntry.
        TTEntry* probe(const Key key, bool& found) const;

        /**
         * The lowest order bits of the key are used to get the index of the cluster. e..g with
         * cluster_count = 2 ^ 9, have cluster_count - 1 = 0b111111111 and we are limited
         * to 2 ^ 9 distinct indices for the clusters.
         */
        TTEntry* first_entry(const Key key) const {
            return &table[(size_t)key & (cluster_count - 1)].entry[0];
        }
    private:
        size_t cluster_count;
        Cluster* table;
        void* mem;
    };



extern TranspositionTable TT;

} // namespace


#endif // TTABLE_H_
