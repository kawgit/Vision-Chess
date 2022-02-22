#pragma once

#include "pos.h"
#include "bits.h"
#include "types.h"

enum Bound : uint8_t {LB, EXACT, UB};
typedef uint8_t Gen;

//2 for hashkey
//2 for eval
//1 for depth
//1 for bound
//2 for move
//1 for gen

struct TTEntry {
	uint32_t hashkey32 = 0;
	Eval eval = 0;
	Bound bound = LB;
	Depth depth = 0;
	Move move = 0;
	Gen gen = 0;


	void save(BB _hashkey, Eval _eval, Bound _bound, Depth _depth, Move _move, Gen _gen);
	void forcesave(BB _hashkey, Eval _eval, Bound _bound, Depth _depth, Move _move, Gen _gen);
};

class TT {
	const static int HASHLENGTH = 10 ;
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