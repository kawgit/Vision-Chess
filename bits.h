#pragma once

#include <stdint.h>

typedef uint64_t BB;

int lsb(BB& n);

int poplsb(BB& n);

int bitcount(BB x);

bool bitAt(BB& n, int k);

int rc(int r, int c);

void print(BB n);

BB getColMask(int c);

BB getRowMask(int r);

BB getBB(int s);

BB randBB();