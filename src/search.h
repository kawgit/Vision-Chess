#pragma once

#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)

#include "pos.h"
#include "timer.h"
#include "tt.h"
#include "types.h"
#include "move.h"
#include <thread>
#include <atomic>
#include <string>
#include <mutex>

using namespace std;

BB perft(Pos &p, Depth depth, bool divide = false);

void perftTest();

struct ThreadInfo {
	int root_ply = 0;
	uint64_t nodes = 0;
	string id = "";
	bool searching = true;
	Depth seldepth = 0;

	ThreadInfo(Pos& p, string id_ = "");
};

class SearchInfo {
	public:

	// assets
	Pos root_pos;
	TT tt;
	vector<ThreadInfo> tis;

	// settings
	bool ponder;
	int num_threads = 0;
	Timestamp max_time = -1;
	Depth max_depth = DEPTH_MAX;
	Depth last_depth_searched = 0;
	
	// runtime metadata variables
	bool is_active = false;
	Timestamp start_time = 0;
	mutex depth_increment_mutex;
	
	private:

	Move cm_hueristic[6][64] = {MOVE_NONE};
	Score hist_hueristic[6][64] = {0};
	int hist_score_max = 0;

	public:

	void launch(bool verbose = true);
	void stop();
	void clear();
	void print_uci_info();
	void worker(ThreadInfo& ti, bool verbose = true);

	inline BB get_nodes() {
		BB nodes = 0;
		for (ThreadInfo& ti : tis) {
			nodes += ti.nodes;
		}
		return nodes;
	}

	inline Depth get_seldepth() {
		Depth seldepth = 0;
		for (ThreadInfo& ti : tis) {
			if (ti.seldepth > seldepth) seldepth = ti.seldepth;
		}
		return seldepth;
	}

	inline void add_failhigh(Pos& pos, Move move) {
		cm_hueristic[pos.ref_mailbox(pos.notturn, get_to(pos.last_move())) - PAWN][get_to(pos.last_move())] = move;
		hist_hueristic[pos.ref_mailbox(pos.turn, get_from(move)) - PAWN][get_to(move)]++;
		hist_score_max++;

		if (hist_score_max == SCORE_MAX) { 
			// this should ensure that hist_scores never overflow, 
			// and should mostly ensure relative values are preserved
			for (int i = 0; i < 6; i++) {
				for (int sq = 0; sq < 64; sq++) {
					hist_hueristic[i][sq] /= 2;
				}
			}
			hist_score_max /= 2;
		}
	}

	inline Move get_cm(Pos& pos) {
		return ((pos.last_move() != MOVE_NONE) ? (cm_hueristic[pos.ref_mailbox(pos.notturn, get_to(pos.last_move())) - PAWN][get_to(pos.last_move())]) : MOVE_NONE);
	}

	inline Score get_hist(Pos& pos, Move move) {
		return hist_hueristic[pos.ref_mailbox(pos.turn, get_from(move)) - PAWN][get_to(move)];
	}
};

Eval search(Pos &p, Depth depth, Eval alpha, Eval beta, ThreadInfo& ti, SearchInfo& si);

Eval qsearch(Pos &p, Eval alpha, Eval beta, ThreadInfo& ti, SearchInfo& si);

void timer(bool& target, Timestamp time);

Eval sea_gain(Pos& pos, Move move, Eval alpha);

Eval static_exchange_search(Pos& pos, Square target_square, Color turn, Eval curr_mat, BB occ, Eval target_piece_eval, Eval alpha, Eval beta);

Move get_best_move(Pos& pos, Depth depth);