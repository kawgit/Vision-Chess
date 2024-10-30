#include <cstring>

#include "bits.h"
#include "move.h"
#include "pos.h"
#include "search.h"
#include "tt.h"

TT::TT(size_t size) {
	num_buckets = size / sizeof(TTBucket);
	buckets = new TTBucket[num_buckets];
	clear();
}

void TT::clear() {
	std::memset(buckets, 0, sizeof(buckets) * num_buckets);
}

TTEntry* TT::probe(BB hashkey, bool& found) {
	
	const size_t bucket_idx = ((hashkey & 0xFFFFFFFF) * num_buckets) >> 32;

	assert(bucket_idx < num_buckets);

	TTBucket* bucket = &buckets[bucket_idx];

	for (size_t i = 0; i < TTBucket::BUCKET_SIZE; i++) {

		TTEntry* entry = &bucket->entries[i];

		if (entry->matches(hashkey)) {
			found = true;
			return entry;
		}

		if (entry->is_empty()) {
			found = false;
			return entry;
		}
		
	}

	found = false;

	TTEntry* worst_entry;
	Score    worst_score = SCORE_MAX;

	for (size_t i = 0; i < TTBucket::BUCKET_SIZE; i++) {

		TTEntry* entry = &bucket->entries[i];
		uint8_t  age   = uint16_t(256) + gen - entry->get_gen();
		Score    score = 4 * age + entry->get_bound() + entry->depth;

		if (score < worst_score) {
			worst_score = score;
			worst_entry = entry;
		}

	}

	return worst_entry;

}

void TT::save(TTEntry* entry, Move move, Eval eval, Depth depth, BB hashkey, Bound bound) {

	// const uint8_t age = uint16_t(256) + gen - entry->get_gen();

	if (   entry->is_empty()
		|| entry->matches(hashkey)
		|| entry->get_gen() < gen
		|| entry->depth < depth) {
		entry->move = move;
		entry->eval = eval;
		entry->depth = std::max(entry->depth, depth);
		entry->hashkey32 = hashkey >> 32;
		entry->set_bound(bound);
		entry->set_gen(gen);
	}

}

void TT::forcesave(TTEntry* entry, Move move, Eval eval, Depth depth, BB hashkey, Bound bound) {
	entry->move = move;
	entry->eval = eval;
	entry->depth = depth;
	entry->hashkey32 = hashkey >> 32;
	entry->set_bound(bound);
	entry->set_gen(gen);
}

size_t TT::hashfull() {
	size_t count = 0;
	size_t viewed = std::min(num_buckets, size_t(1000));
	for (size_t i = 0; i < viewed; i++) {
		for (size_t j = 0; j < TTBucket::BUCKET_SIZE; j++) {
			if (buckets[i].entries[j].hashkey32)
				count++;
		}
	}
	return count * 1000 / (viewed * TTBucket::BUCKET_SIZE);
}

std::vector<Move> TT::probe_pv(Pos& pos) {
	std::vector<Move> pv;
	add_pv(pos, pv);
	return pv;
}

void TT::add_pv(Pos& pos, std::vector<Move>& pv) {

	if (pos.three_repetitions())
		return;

	bool found = false;
	const TTEntry* entry = probe(pos.hashkey(), found);

	if (!found)
		return;

	std::vector<Move> legal_moves = movegen::generate<LEGAL>(pos);
	for (Move legal_move : legal_moves) {
		if (legal_move == entry->move) {
			pv.push_back(legal_move);

			pos.do_move(legal_move);

			add_pv(pos, pv);

			pos.undo_move();

			return;
		}
	}

	

}