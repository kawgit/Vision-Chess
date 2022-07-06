#include "bits.h"
#include "pos.h"
#include "hash.h"
#include "types.h"
#include <cstdlib>

using namespace std;

BB z_squares_[2][6][64] = {};
BB z_ep_[9] = {}; //one for each column and the last one for no EP
BB z_cr_[4] = {};
BB z_turn_ = 0;

void initHash(int seed) {
    srand(seed);
    for (int i = PAWN; i <= KING; i++) {
        for (int s = 0; s < 64; s++) {
            z_squares(WHITE, i, s) = rand_BB();
            z_squares(BLACK, i, s) = rand_BB();
        }
    }

    for (int i = 0; i < 9; i++) z_ep(i) = rand_BB();
    for (int i = 0; i < 4; i++) z_cr((CR_Index)i) = rand_BB();
    z_turn() = rand_BB();
}