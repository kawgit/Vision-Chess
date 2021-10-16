#include "zobrist.h"
#include "types.h"
#include "util.h"
#include "pos.h"
#include <iostream>

using namespace std;

BB z_white_squares[6][64] = {};
BB z_black_squares[6][64] = {};
BB z_ep[8] = {}; //one for each column
BB z_cr[4] = {};
BB z_turn = {};

void initZ(int seed) {
    srand(seed);
    for (int i = 0; i < 6; i++) {
        for (int s = 0; s < 64; s++) {
            z_white_squares[i][s] = randBB();
            z_black_squares[i][s] = randBB();
        }
    }

    for (int i = 0; i < 8; i++) z_ep[i] = randBB();
    for (int i = 0; i < 4; i++) z_cr[i] = randBB();
    z_turn = randBB();
}

BB _hash(Pos p) {
    BB key = 0ULL;

    for (int i = 0; i < 6; i++) {
        if (p.white_pieces[i] != 0) {
            BB save = p.white_pieces[i];
            while (save) {
                int s = poplsb(save);
                key ^= z_white_squares[i][s];
            }
        }
        if (p.black_pieces[i] != 0) {
            BB save = p.black_pieces[i];
            while (save) {
                int s = poplsb(save);
                key ^= z_black_squares[i][s];
            }
        }
    }

    if (p.cr.getWK()) key ^= z_cr[0];
    if (p.cr.getWQ()) key ^= z_cr[1];
    if (p.cr.getBK()) key ^= z_cr[2];
    if (p.cr.getBQ()) key ^= z_cr[3];

    if (p.ep < 64) key ^= z_ep[p.ep%8];

    if (p.turn == WHITE) key ^= z_turn;

    return key;
}