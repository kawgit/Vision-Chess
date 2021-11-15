#include "tt.h"
#include "types.h"
#include <iostream>

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

void TT::clear() {
    for (int i = 0; i < TT::TABLE_SIZE; i++) {
        for (int j = 0; j < TTGroup::GROUP_SIZE; j++) {
            table[i].entries[j].save(0, Move(), 0, 0, -1, LB);
        }
    }
}

int TT::hashfull() {
    int count = 0;
    for (int i = 0; i < 1000/TTGroup::GROUP_SIZE; i++) {
        for (int j = 0; j < TTGroup::GROUP_SIZE; j++) {
            if (table[i].entries[j].key) count++;
        }
    }
    return count;
}

vector<Move> TT::getPV(Pos p) {
    vector<Move> PV;
    while (true) {
        bool found = false;
        TTEntry* entry = getEntry(p.key, found);
        Move move = entry->move;
        if (found) {
            PV.push_back(move);
            p.makeMove(move);
        }
        else break;
    }
    return PV;
}