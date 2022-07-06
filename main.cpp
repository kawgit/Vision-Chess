#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include "pos.h"
#include "bits.h"
#include "search.h"
#include "timer.h"
#include "search.h"
#include "types.h"
#include "movegen.h"
#include "eval.h"

using namespace std;


int main() {
	initHash(23232);
	initMoveGen();

	Pos pos("r2qr1k1/p3n1bR/1p1p2p1/2pb4/2P2PP1/3B1N2/PPQ2PK1/R7 w - - 0 20");

	cout << "root: " << to_string(evalPos(pos, -INF, INF)) << endl;

	Timestamp start = get_current_ms();

	ThreadInfo ti(pos);
	SearchInfo si;

	for (int depth = 0; depth <= 20; depth++) {
		Eval eval = search(pos, depth, -INF, INF, ti, si);
		cout << "tree: " << to_string(depth) << " cp " << eval_to_string(eval) << " nodes " << to_string(ti.nodes) << " time " << to_string(get_time_diff(start)) << " pv " << to_string(si.tt.getPV(pos)) << endl;
	}

	print_time_diff(start);

}