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
#include "util.h"
#include "nnue.h"



int main() {

	zobrist::init();
	attacks::init();

	nnue::read_network("../scripts/kwnnue.kwnnue");

	Pos pos("r1bq1rk1/ppp2ppp/3n4/8/8/2N1B3/PPP2PPP/R2QRBK1 b - - 1 14");

	float eval = nnue::evaluate(pos.turn());
	std::cout << eval << std::endl;

	print(pos, true);

	perft<true>(pos, 5);

	return 0;
}