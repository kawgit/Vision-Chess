#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include "pos.h"
#include "move.h"
#include "attacks.h"
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
	attacks::init();
	
	Pos pos("rnbqk1nr/pppp1p1p/3bp1p1/8/4P3/3B1N2/PPPP1PPP/RNBQK2R w KQkq - 0 4");
	Move move = make_move(E1, G1, KING_CASTLE);

	print(pos);
	
	cout << "do " << move_to_string(move) << endl;
	pos.do_move(move);

	print(pos);
	
	cout << "undo " << move_to_string(move) << endl;
	pos.undo_move();
	
	print(pos);
	
	print(attacks::pawn(D4, BLACK));
	print(attacks::knight(D4));
	print(attacks::bishop(D4, pos.pieces()));
	print(attacks::rook(D4, pos.pieces()));
	print(attacks::queen(D4, pos.pieces()));
	print(attacks::king(D4));
	// init_hash(4643);
	// init_movegen();

	// uci();

	cout << "EXIT SUCCESS" << endl;

	return 0;
}