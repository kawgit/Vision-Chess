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

    Pos p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    print(p);

    for (int i = 1; i < 8; i++) {
        cout<<to_string(perft(p, i, false))<<endl;
    }

    print(p);

    return 0;
}