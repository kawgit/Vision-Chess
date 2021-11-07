#include "types.h"
#include "util.h"

BB square_masks[64] = {};
BB file_masks[8] = {};
BB rank_masks[8] = {};


void initBB() {
    const BB FILE = 0b0000000100000001000000010000000100000001000000010000000100000001;
    const BB RANK = 0b0000000000000000000000000000000000000000000000000000000011111111;

    for (int i = 0; i != 64; i++) {
        square_masks[i] = 1ULL<<i;
    }

    for (int i = 0; i != 8; i++) {
        file_masks[i] = FILE<<i;
    }

    for (int i = 0; i != 8; i++) {
        rank_masks[i] = RANK<<i*8;
    }
}

string pnot[6] = {"p","n","b","r","q","k"};
string Move::getSAN() {
    return getSquareN(getFr()) + getSquareN(getTo()) + (isPromotion() ? pnot[getProm()] : "");
}

Color getOppositeColor(Color c) {
    return (c == WHITE ? BLACK : WHITE);
}