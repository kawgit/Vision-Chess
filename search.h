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

struct SearchInfo {
	TT* tt;
	Move cm_hueristic[6][64] = {MOVENONE};
	uint32_t hist_hueristic[6][64] = {0};
};

struct ThreadInfo {
	int root_ply = 0;
	int nodes = 0;
	bool searching = true;
	ThreadInfo(Pos& p) {
		root_ply = p.m_clock;
	}
};

Eval search(Pos &p, Depth depth, SearchInfo* searchInfo = nullptr);

Eval search(Pos &p, Depth depth, Eval alpha, Eval beta, ThreadInfo& threadInfo, SearchInfo* searchInfo = nullptr);

Eval qsearch(Pos &p, Eval alpha, Eval beta, ThreadInfo& threadInfo, SearchInfo* searchInfo = nullptr);

vector<Move> order(SearchInfo* searchInfo, Pos& pos, vector<Move> unsorted_moves, Move entry_move);