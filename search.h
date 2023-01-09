#pragma once

#include "pos.h"
#include "timer.h"
#include "tt.h"
#include "types.h"
#include <thread>
#include <atomic>
#include <string>
#include <mutex>

using namespace std;

#ifndef _WIN32
#include <unistd.h>
#define sleep(ms) usleep(ms * 1000)
#endif
#ifdef _WIN32
#include <Windows.h>
#define sleep(ms) Sleep(ms)
#endif

BB perft(Pos &p, Depth depth, bool divide = false);

void perftTest();

struct ThreadInfo {
	int root_ply = 0;
	uint64_t nodes = 0;
	string id = "";
	bool searching = true;

	ThreadInfo(Pos& p, string id_);
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
	Depth max_depth = DEPTHMAX;
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
	void print();
	void worker(ThreadInfo& ti, bool verbose = true);

	inline int get_nodes() {
		BB nodes = 0;
		for (ThreadInfo& ti : tis) {
			nodes += ti.nodes;
		}
		return nodes;
	}

	inline void add_failhigh(Pos& pos, Move move) {
		cm_hueristic[pos.mailboxes(pos.notturn, get_to(pos.last_move())) - PAWN][get_to(pos.last_move())] = move;
		hist_hueristic[pos.mailboxes(pos.turn, get_from(move)) - PAWN][get_to(move)]++;
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
		return ((pos.last_move() != MOVE_NONE) ? (cm_hueristic[pos.mailboxes(pos.notturn, get_to(pos.last_move())) - PAWN][get_to(pos.last_move())]) : MOVE_NONE);
	}

	inline Score get_hist(Pos& pos, Move move) {
		return hist_hueristic[pos.mailboxes(pos.turn, get_from(move)) - PAWN][get_to(move)];
	}
};

Eval search(Pos &p, Depth depth, Eval alpha, Eval beta, ThreadInfo& ti, SearchInfo& si);

Eval qsearch(Pos &p, Eval alpha, Eval beta, ThreadInfo& ti, SearchInfo& si);

void timer(bool& target, Timestamp time);