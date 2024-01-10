#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include "attacks.h"
#include "move.h"
#include "movegen.h"
#include "pos.h"
#include "util.h"

using namespace std;

int main() {
	zobrist::init();
	attacks::init();
	
	Pos pos;

	vector<Move> moves = movegen::generate<movegen::PSEUDO>(pos);

	for (Move move : moves) {
		cout << move_to_string(move) << endl;
	}

	cout << moves.size() << endl;

	return 0;
}