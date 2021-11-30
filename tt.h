#include "pos.h"
#include "types.h"

enum Bound_Flag {LB, UB, EXACT};

struct TTEntry {
    BB key = 0;
    Move move;
    Eval eval = 0;
    Depth depth = 0;
    Bound_Flag bound = LB;

    void save(BB key_, Move move_, Eval eval_, Depth depth_, Bound_Flag bound_);
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
    void clear();
    void reset();
    int hashfull();
    vector<Move> getPV(Pos p);

    public:
    TTGroup table[TABLE_SIZE];
};

