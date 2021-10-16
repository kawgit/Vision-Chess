#pragma once

#include "types.h"
#include "pos.h"

void print(Pos p);

void print(BB b);

inline int RC2SQ(int r, int c) { return r*8+c;}

inline bool bitAt(BB b, int i) { return b & (1ULL << i);};

string getSquareN(Square s);

inline BB randBB() { return ((long long)rand() << 32) | rand();};

int lsb(BB n);

int poplsb(BB &n);