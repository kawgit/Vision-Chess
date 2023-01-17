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
#include "uci.h"
#include "order.h"

using namespace std;


int main(int argc, char* argv[]) {
	init_hash(4643);
	init_movegen();

	uci();

	// SearchInfo si;
	// si.root_pos = Pos("rnbq1rk1/2pp1ppp/pp2pn2/4P1B1/1b1P4/2N2N2/PPP2PPP/R2QKB1R w KQ - 0 8");
	// ThreadInfo ti(si.root_pos, "hi");
	// vector<Move> moves = get_legal_moves(si.root_pos);
	// int interesting = 0;
	// moves = order(moves, si.root_pos, ti, si, interesting, false);
	// cout << to_string(moves) << endl;
	
	// search(si.root_pos, 8, -INF, INF, ti, si);

	cout << "EXIT SUCCESS" << endl;
}