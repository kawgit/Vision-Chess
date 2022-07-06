#pragma once

#include "pos.h"
#include "bits.h"
#include "types.h"

enum Bound : uint8_t {LB, EXACT, UB};
typedef uint8_t Gen;

//16 bits for eval
//16 bits for move
//8 bits for depth
//8 bits for gen
//2 bits for bound

//32 bits for hashkey

struct TTEntry {
	uint64_t s1 = 0;
	uint32_t s2 = 0;

	inline Eval get_eval() { return (s1) & 0xFFFF; }
	inline Move get_move() { return (s1 >> 16) & 0xFFFF; }
	inline Depth get_depth() { return (s1 >> 32) & 0xFF; }
	inline Gen get_gen() { return (s1 >> 40) & 0xFF; }
	inline Bound get_bound() { return (Bound)((s1 >> 48) & 0b11); }
	inline uint32_t get_hashkey32() { return s2; }

	inline void set_eval(BB eval)   { s1 &=            0xFFFFULL; s1 |= (eval & 0xFFFF); }
	inline void set_move(BB move)   { s1 &=       ~0xFFFF0000ULL; s1 |= (move & 0xFFFF) << 16; }
	inline void set_depth(BB depth) { s1 &=     ~0xFF00000000ULL; s1 |= (depth & 0xFF) << 32; }
	inline void set_gen(BB gen)     { s1 &=   ~0xFF0000000000ULL; s1 |= (gen & 0xFF) << 40; }
	inline void set_bound(BB bound) { s1 &= ~0xFF000000000000ULL; s1 |= (bound & 0b11) << 48; }
	inline void set_hashkey32(BB hashkey) { s2 = hashkey >> 32; }

	inline bool matches(BB hashkey) { return get_hashkey32() == (hashkey >> 32); }

	inline void save(BB hashkey, Eval eval, Bound bound, Depth depth, Move move, Gen gen) {
		if ( gen > get_gen() ||
			(get_bound() != EXACT && (bound == EXACT || get_depth() < depth)) ||
			(get_bound() == EXACT && (bound == EXACT && get_depth() < depth))) {
			forcesave(hashkey, eval, bound, depth, move, gen);
		}
	}
	
	inline void forcesave(BB hashkey, Eval eval, Bound bound, Depth depth, Move move, Gen gen) {
		set_hashkey32(hashkey);
		set_eval(eval);
		set_bound(bound);
		set_depth(depth);
		set_move(move);
		set_gen(gen);
	}

	inline void set_zero() {
		s1 = 0;
		s2 = 0;
	}
	
};

class TT {
	const static int HASHLENGTH = 25 ;
	const static BB TABLESIZE = (1ULL<<HASHLENGTH);
	const static BB HASHMASK = TABLESIZE-1;
	const static int tableSizeInMb = TABLESIZE*sizeof(TTEntry)/0x100000;
	
	public:
	TTEntry* probe(BB hashkey, bool& found);

	void clear();
	vector<Move> getPV(Pos p);
	void addPV(Pos& p, vector<Move>& pv);

	Gen gen = 0;
	vector<TTEntry> table;

	TT() : table(TABLESIZE) {}
};