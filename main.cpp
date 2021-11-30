#include "pos.h"
#include "util.h"
#include "types.h"
#include "zobrist.h"
#include "movegen.h"
#include "search.h"
#include <iostream>

int main() {
    initBB();
    initZ(3333333);
    initM();

    cout.setf(ios::unitbuf);

    Pos p("r3k2r/ppp2ppp/1q2p3/3p2P1/1b1Pb3/2B1PN1P/PPP2P2/1R1QK2R w Kkq - 2 14");

    print(p);


    bool ongoing = true;
    while (ongoing) {
        Search s(p);

        s.max_ms = 5000;

        s.max_depth = 9;
        s.table.clear();
        s.go();
        cout<<s.root_bestmove.getSAN()<<endl;
        p.makeMove(s.root_bestmove);

        vector<Move> moves;
        addLegalMoves(p, moves);
        if (moves.size() == 0) ongoing = false;
    
        cout<<p.getPGN()<<endl;
        print(p);
        break;
    }

    return 0;
}