#include "pos.h"
#include "util.h"
#include "types.h"
#include "zobrist.h"
#include "movegen.h"
#include "search.h"
#include <iostream>

int main() {
    initBB();
    initZ();
    initM();

    cout.setf(ios::unitbuf);

    Pos p("r3r1k1/1p2pp2/3pb1p1/3N2P1/2PbP3/pP3P2/P3K2R/7R w - - 1 23");


    print(p);

    Search s(p);
    s.max_depth = 18;
    s.go();

    return 0;
}