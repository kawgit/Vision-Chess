#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include "pos.h"
// #include "bits.h"
// #include "search.h"
// #include "timer.h"
// #include "search.h"
// #include "types.h"
// #include "movegen.h"
// #include "eval.h"
// #include "uci.h"
// #include "order.h"
// #include "train.h"

using namespace std;


int main(int argc, char* argv[]) {
	zobrist::init();

	
	Pos pos;
	print(pos);
	print(pos.pieces(WHITE, PAWN));
	print(pos.pieces(BLACK, PAWN));
	print(pos.pieces(PAWN));
	print(pos.pieces(BLACK));
	
	// init_hash(4643);
	// init_movegen();

	// uci();

	cout << "EXIT SUCCESS" << endl;

	return 0;
}