#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include "attacks.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "pos.h"
#include "search.h"
#include "movepicker.h"
#include "util.h"
// #include "nnue.h"
// #include "thread.h"



int main() {

	zobrist::init();
	attacks::init();

	// Pos pos;

	// perft<true>(pos, 6);

	Pos pos("3q1rk1/4ppbp/p1bp1np1/8/1r2P1PP/4BP2/NPPQB3/1K1R3R b - - 1 17");

	print(pos, true);




	// std::vector<Move> moves = movegen::generate<LEGAL>(pos);

	// History history;
	// MovePicker mp(&pos, moves, MOVE_NONE, &history);


	// while (mp.has_move()) {

	// 	Move move = mp.pop();

	// 	std::cout << "currmove " << move_to_string(move) << " mp stage " << int(mp.stage) << std::endl;
	// }




	// for (Piece piece = PAWN; piece <= KING; piece++) {
	// 	print(mp.unsafe_bbs[piece] & ~pos.pieces(pos.turn()));
	// }

	// for (Color color : {WHITE, BLACK}) {
	// 	print(mp.hanging_bbs[color]);
	// }

	Pool pool(1, 64*1000000);

	pool.reset(pos);

	pool.wake();

	sleep_ms(6000);

	// Accumulator acc;
	// acc.reset(pos);

	// std::cout << nnue::evaluate(acc, WHITE) << std::endl;
	// std::cout << nnue::evaluate(acc, BLACK) << std::endl;

	// pos = Pos("rnbqkbnr/pppppppp/8/8/8/3P4/PPP1PPPP/RNBQKBNR w KQkq - 0 1");

	// acc.reset(pos);

	// std::cout << nnue::evaluate(acc, WHITE) << std::endl;
	// std::cout << nnue::evaluate(acc, BLACK) << std::endl;


	// print(pos, true);

	// Pool pool(1, 1<<8);

	// perft<true>(pos, 6);

	// pool.wake();

	// perft<true>(pos, 6);
	// perft<true>(pos, 6);
	// perft<true>(pos, 6);

	return 0;
}