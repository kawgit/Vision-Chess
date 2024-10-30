#pragma once

#include <algorithm>

#include "bits.h"
#include "move.h"
#include "pos.h"
#include "types.h"

struct TTEntry {
    Move move;
    Eval eval;
    Depth depth;
	uint32_t hashkey32;
	uint8_t genbound;

	inline Gen get_gen()     const { return genbound & 0b111111; }
	inline Bound get_bound() const { return genbound >> 6;}

	inline void set_gen(Gen gen)       { genbound &= ~0b00111111; genbound |= gen; }
	inline void set_bound(Bound bound) { genbound &= ~0b11000000; genbound |= bound << 6; }

	inline bool matches(BB hashkey) { return hashkey32 == hashkey >> 32; }
    inline bool is_empty()          { return !move; }
	
};

struct TTBucket {
    static const size_t BUCKET_SIZE = 3;
    TTEntry entries[BUCKET_SIZE];
};

class TT {

    private:

        TTBucket* buckets;
        size_t num_buckets;

    public:

        Gen gen = 0;
	    TT(size_t size);
        void clear();
        TTEntry* probe(BB hashkey, bool& found);
        void forcesave(TTEntry* entry, Move move, Eval eval, Depth depth, BB hashkey, Bound bound);
        void save     (TTEntry* entry, Move move, Eval eval, Depth depth, BB hashkey, Bound bound);
        size_t hashfull();
        std::vector<Move> probe_pv(Pos& pos);

    private:

        void add_pv(Pos& pos, std::vector<Move>& pv);


};
