#include "bits.h"
#include "pos.h"
#include <cstdlib>

using namespace std;

BB z_squares[2][6][64] = {};
BB z_ep[9] = {}; //one for each column and the last one for no EP
BB z_cr[4] = {};
BB z_turn = 0;

void initHash(int seed) {
    srand(seed);
    for (int i = 0; i < 6; i++) {
        for (int s = 0; s < 64; s++) {
            z_squares[0][i][s] = randBB();
            z_squares[1][i][s] = randBB();
        }
    }

    for (int i = 0; i < 9; i++) z_ep[i] = randBB();
    for (int i = 0; i < 4; i++) z_cr[i] = randBB();
    z_turn = randBB();
}