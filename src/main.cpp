#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include "pos.h"
#include "move.h"
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


int main() {
	zobrist::init();

	
	Pos pos("rnbqk1nr/pppp1p1p/3bp1p1/8/4P3/3B1N2/PPPP1PPP/RNBQK2R w KQkq - 0 4");
	print(pos);

	Move move = make_move(E1, G1, KING_CASTLE);
	
	cout << move_to_string(move) << endl;

	pos.do_move(move);
	print(pos);
	
	// init_hash(4643);
	// init_movegen();

	// uci();

	cout << "EXIT SUCCESS" << endl;

	return 0;
}