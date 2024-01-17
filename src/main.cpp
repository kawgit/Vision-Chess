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



int main() {

	zobrist::init();
	attacks::init();
	
	Pos pos("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ");

	print(pos, true);

	perft<true>(pos, 5);

	return 0;
}