#include "pos.h"
#include "util.h"
#include "types.h"
#include "zobrist.h"
#include <iostream>

int main() {
    initBB();
    initZ();

    
    Pos p;

    print(p);

    return 0;
}