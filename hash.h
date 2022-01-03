#pragma once

#include "types.h"
#include "bits.h"
#include "pos.h"

using namespace std;


extern BB z_squares[2][6][64];
extern BB z_ep[9]; //one for each column than one for no ep
extern BB z_cr[4];
extern BB z_turn;

void initHash(int seed = 696969);