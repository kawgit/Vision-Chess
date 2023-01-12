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

	cout << "READY" << endl;


	uci();

	// SearchInfo si;
	// si.root_pos = Pos("r1bqk1nr/ppppppbp/8/4n1p1/8/4QN2/PPPPPPPP/RNB1KB1R w KQkq - 0 2");
	// ThreadInfo ti(si.root_pos, "hi");
	// cout << to_string(si.root_pos.ref_hashkey()) << endl;
	// search(si.root_pos, 8, -INF, INF, ti, si);

	cout << "EXIT SUCCESS" << endl;
}