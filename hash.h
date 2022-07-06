#pragma once

#include "types.h"
#include "bits.h"
#include "pos.h"

using namespace std;

extern BB z_squares_[2][6][64];
extern BB z_ep_[9]; //one for each column than one for no ep
extern BB z_cr_[4];
extern BB z_turn_;

inline BB& z_squares(Color color, Piece piece, Square square) {
    return z_squares_[color - BLACK][piece - PAWN][square];
}

inline BB& z_ep(File file) {
    return z_ep_[file];
}

inline BB& z_cr(CR_Index i) {
    return z_cr_[i];
}

inline BB& z_turn() {
    return z_turn_;
}


void initHash(int seed);