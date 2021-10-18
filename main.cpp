#include "pos.h"
#include "util.h"
#include "types.h"
#include "zobrist.h"
#include "movegen.h"
#include <iostream>

int main() {
    initBB();
    initZ();
    initM();

    Pos p;

    print(p);

    return 0;
}