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
	// si.root_pos = Pos("8/2p3pk/3r3p/p1p2R2/P1P5/2P1q1P1/7P/5K2 b - - 0 35");
	// ThreadInfo ti(si.root_pos, "hi");
	// vector<Move> moves = get_legal_moves(si.root_pos);
	// int num_good;
	// int num_boring;
	// int num_bad;
	// moves = order(moves, si.root_pos, ti, si, num_good, num_boring, num_bad, false);
	// cout << to_string(moves) << endl;
	
	// search(si.root_pos, 8, -INF, INF, ti, si);

	cout << "EXIT SUCCESS" << endl;
}