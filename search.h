#pragma once

#include "pos.h"
#include "timer.h"
#include "tt.h"
#include "types.h"
#include <thread>
#include <atomic>

using namespace std;

BB perft(Pos &p, Depth depth, bool divide = false);

void perftTest();

class SearchInfo {
	public:

	TT tt;
	
	private:

	Move cm_hueristic[6][64] = {MOVE_NONE};
	Score hist_hueristic[6][64] = {0};
	int hist_score_max = 0;

	public:

	inline void add_failhigh(Pos pos, Move move) {
		get_cm(pos) = move;
		get_hist(pos, move)++;
		hist_score_max++;

		if (hist_score_max == SCORE_MAX) { 
			// this should ensure that hist_scores never overflow, 
			// and should mostly ensure order is preserved
			for (int i = 0; i < 6; i++) {
				for (int sq = 0; sq < 64; sq++) {
					hist_hueristic[i][sq] /= 2;
				}
			}
			hist_score_max /= 2;
		}
	}

	inline Move& get_cm(Pos pos) {
		return cm_hueristic[pos.mailboxes(pos.notturn, get_to(pos.last_move())) - PAWN][get_to(pos.last_move())];
	}

	inline Score& get_hist(Pos pos, Move move) {
		return hist_hueristic[pos.mailboxes(pos.turn, get_from(move)) - PAWN][get_to(move)];
	}
};

struct ThreadInfo {
	int root_ply = 0;
	int nodes = 0;
	bool searching = true;
	ThreadInfo(Pos& p) {
		root_ply = p.m_clock;
	}
};

Eval search(Pos &p, Depth depth, Eval alpha, Eval beta, ThreadInfo& ti, SearchInfo& si);

Eval qsearch(Pos &p, Eval alpha, Eval beta, ThreadInfo* ti = nullptr, SearchInfo* si = nullptr);
