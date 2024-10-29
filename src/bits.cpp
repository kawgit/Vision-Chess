#include "bits.h"
#include "util.h"
#include <stdint.h>
#include <iostream>
#include <cstdlib>

void print(BB bb) {
	
	for (Rank rank = RANK_8; rank >= RANK_1; rank--) {

		for (File file = FILE_A; file <= FILE_H; file++) {

			std::cout << (bb_has(bb, square_of(rank, file)) ? "O" : "-") << " ";

		}

		std::cout << std::endl;

	}

	std::cout << std::endl;

}