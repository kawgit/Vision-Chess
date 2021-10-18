#pragma once
#include "types.h"

const int ROOK_BITS = 16;
const int BISHOP_BITS = 11;

const int ROOK_SHIFT = 64-ROOK_BITS;
const int BISHOP_SHIFT = 64-BISHOP_BITS;

extern BB bishop_magics[64];
extern BB rook_magics[64];

extern BB bishop_table[64][1<<BISHOP_BITS];
extern BB rook_table[64][1<<ROOK_BITS];

extern BB rays[64][8];

extern BB rook_moves[64];
extern BB bishop_moves[64];

extern BB rook_blockermasks[64];
extern BB bishop_blockermasks[64];


void initM(int seed = 32123);