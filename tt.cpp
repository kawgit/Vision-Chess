#include "tt.h"
#include "types.h"

TTEntry* TT::getEntry(BB key, bool &found) {
    TTGroup& group = table[key & HASH_MASK];

    TTEntry* usable = &group.entries[0];
    bool usable_found = false;
    int min_depth_found = 500;

    for (int i = 0; i < TTGroup::GROUP_SIZE; i++) {
        if (group.entries[i].key == key) {
            found = true;
            return &group.entries[i];
        }
        
        if (!usable_found) {
            if (group.entries[i].mclock < mclock_threshold) {
                usable_found = true;
                usable = &group.entries[i];
            }
            else if (group.entries[i].depth < min_depth_found) {
                min_depth_found = group.entries[i].depth;
                usable = &group.entries[i];
            }
        }
    }

    found = false;

    return usable;
}