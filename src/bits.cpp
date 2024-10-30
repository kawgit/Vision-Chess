#include <cstdlib>
#include <iostream>
#include <stdint.h>

#include "bits.h"
#include "util.h"

void print_bb(BB bb) {
	
	for (Rank rank = RANK_8; rank >= RANK_1; rank--) {

		for (File file = FILE_A; file <= FILE_H; file++) {

			std::cout << (bb_has(bb, square_of(rank, file)) ? "O" : "-") << " ";

		}

		std::cout << std::endl;

	}

	std::cout << std::endl;

}