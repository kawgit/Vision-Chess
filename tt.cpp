#include "pos.h"
#include "bits.h"
#include "tt.h"
#include "search.h"

void TTEntry::save(BB _hashkey, Eval _eval, Bound _bound, Depth _depth, Move _move, Gen _gen) {
	if ( _gen > gen ||
		(bound != EXACT && (_bound == EXACT || depth < _depth)) ||
		(bound == EXACT && (_bound == EXACT && depth < _depth))) {
		hashkey32 = _hashkey>>32;
		eval = _eval;
		bound = _bound;
		depth = _depth;
		move = _move;
		gen = _gen;
	}
}

void TTEntry::forcesave(BB _hashkey, Eval _eval, Bound _bound, Depth _depth, Move _move, Gen _gen) {
	hashkey32 = _hashkey>>32;
	eval = _eval;
	bound = _bound;
	depth = _depth;
	move = _move;
	gen = _gen;
}

TTEntry* TT::probe(BB hashkey, bool& found) {
	TTEntry* entry = &table[hashkey & HASHMASK];
	found = entry->hashkey32 == hashkey>>32;
	return entry;
}

void TT::clear() {
	gen = 0;
	for (int i = 0; i < TABLESIZE; i++) {
		table[i].forcesave(0, 0, LB, 0, 0, 0);
	}
}


vector<Move> TT::getPV(Pos p) {
	vector<Move> pv;
	addPV(p, pv);
	return pv;
}

void TT::addPV(Pos& p, vector<Move>& pv) {
	bool found = false;
	TTEntry* entry = probe(p.hashkey, found);
	Move move = entry->move;
	Bound bound = entry->bound;
	if (found && bound == EXACT) {
		p.makeMove(move);
		pv.push_back(move);
		if (!p.isGameOver()) addPV(p, pv);
		p.undoMove();
	}
}