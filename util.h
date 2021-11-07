#pragma once

#include "types.h"
#include "pos.h"


void print(Pos p);

void print(BB b);

inline int RC2SQ(int r, int c) { return r*8+c;}

inline bool bitAt(BB b, int i) { return b & getBB(i);};

string getSquareN(Square s);

inline BB randBB() {
    unsigned long long r = 0;
    for (int i = 0; i < 5; ++i) r = (r << 15) | (rand() & 0x7FFF);
    return r & 0xFFFFFFFFFFFFFFFFULL;
};

int lsb(BB n);

int poplsb(BB &n);

int bitcount(BB x);

unsigned int getCurrentMs();