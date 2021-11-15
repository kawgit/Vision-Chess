#include "pos.h"
#include "types.h"

enum Bound_Flag {LB, UB, EXACT};

struct TTEntry {
    BB key = 0;
    Move move;
    Eval eval = 0;
    Depth depth = 0;
    int mclock = -1; //how far through the game was this entry made? used to check for entries made before root node
    Bound_Flag bound = LB;

    void save(BB key_, Move move_, Eval eval_, Depth depth_, int mclock_, Bound_Flag bound_) {
        key = key_;
        move = move_;
        eval = eval_;
        depth = depth_;
        mclock = mclock_;
        bound = bound_;
    }
};

struct TTGroup {
    const static int GROUP_SIZE = 3;
    TTEntry entries[GROUP_SIZE];
};

class TT {
    const static int HASH_SIZE = 10;
    const static int TABLE_SIZE = 1<<HASH_SIZE;
    const static BB HASH_MASK = (1<<HASH_SIZE) - 1;
    
    public:
    TTEntry* getEntry(BB key, bool &found);

    public:
    int mclock_threshold = 0; //entries with mclock less than mclock_threshold will be disregarded
    TTGroup table[TABLE_SIZE];
};

