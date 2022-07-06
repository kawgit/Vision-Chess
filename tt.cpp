#include "pos.h"
#include "bits.h"
#include "tt.h"
#include "search.h"

TTEntry* TT::probe(BB hashkey, bool& found) {
	TTEntry* entry = &table[hashkey & HASHMASK];
	found = entry->matches(hashkey);
	return entry;
}

void TT::clear() {
	gen = 0;
	for (int i = 0; i < TABLESIZE; i++) {
		table[i].set_zero();
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
	Move move = entry->get_move();
	Bound bound = entry->get_bound();
	if (found && bound == EXACT) {
		p.do_move(move);
		pv.push_back(move);
		if (!p.is_over()) addPV(p, pv);
		p.undo_move();
	}
}