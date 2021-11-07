#include "search.h"
#include "util.h"
#include "pos.h"
#include "types.h"
#include "movegen.h"
#include "zobrist.h"
#include "timer.h"
#include <iostream>

BB perft(Pos &p, int depth, bool divide) {
    if (depth == 0) return 1;

    vector<Move> moves = getLegalMoves(p);
    Timestamp start;
    if (divide) {
        start = get_current_ms();
    }

    BB count = 0;
    for (Move m : moves) {
        //BB save = _hash(p);
        p.makeMove(m);
        int c = perft(p, depth-1, false);
        p.undoMove();

        if (divide) cout<<m.getSAN()<<": "<<to_string(c)<<endl;
        count += c;
    }

    if (divide) {
        cout<<"total: "<<to_string(count)<<endl;
        print_time_diff(start);
        cout<<"NPS: "<<to_string((int)(((double)count/(get_current_ms()-start))*1000))<<endl;
    }
    return count;
}