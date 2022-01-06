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

void puzzleTest(int min_time = 1000, int maxtime = 10000);

class Search {
	public:
	
	Search(Pos p);

	void go();
	void stop();
	void manager();
	void limitsChecker();
	Eval search(Pos &p, Depth depth, Eval alpha, Eval beta);
	Eval qsearch(Pos &p, Eval alpha, Eval beta);

	public:
	//settings

	bool useHashTable = true;
	bool futilityPruning = true;
	Eval futilityMargin = 900;
	int num_threads = 1;

	//info

	Timestamp search_start = 0;
	atomic<Move> root_bestmove;
	Pos root_pos;
	vector<Move> root_moves;
	bool searching = false;
	atomic_int min_thread_depth{1};
	atomic_int max_thread_depth{1};
	TT tt;

	//limits
	Depth max_depth = DEPTHMAX;

	Timestamp wtime = 0;
	Timestamp btime = 0;
	Timestamp winc = 0;
	Timestamp binc = 0;
	bool infinite = false;
	bool ponder = false;

	//hueristics
	Move cm_hueristic[6][64] = {MOVENONE}; //previous move (fromPiece, toSquare)
	unsigned int hist_hueristic[6][64] = {0}; //fromPiece, toSquare
};

struct WorkerThread {
	Pos root_pos;
	Depth root_depth = 0;
	Search* parent;
	BB nodes = 0;

	WorkerThread(Search& search);
	void start();
	Eval search(Pos &p, Depth depth, Eval alpha, Eval beta);
	Eval qsearch(Pos &p, Eval alpha, Eval beta);
};