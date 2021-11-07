#include "types.h"
#include "pos.h"


extern BB z_white_squares[6][64];
extern BB z_black_squares[6][64];
extern BB z_ep[8]; //one for each column than one for no ep
extern BB z_cr[4];
extern BB z_turn;

void initZ(int seed = 69649385);

BB _hash(Pos p);
