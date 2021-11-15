#include "search.h"
#include "util.h"
#include "pos.h"
#include "types.h"
#include "movegen.h"
#include "zobrist.h"
#include "timer.h"
#include "eval.h"
#include <iostream>

using namespace std;

BB perft(Pos &p, Depth depth, bool divide) {
    if (depth == 0) return 1;

    vector<Move> moves;
    addLegalMoves(p, moves);

    Timestamp start;
    if (divide) {
        start = get_current_ms();
    }

    BB count = 0;
    for (Move m : moves) {

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

Search::Search(Pos &p) {
    cout<<"hey"<<endl;
    root_p = p;
    root_move_clock = p.move_clock;
}

void Search::go() {
    searching = true;
    Pos p = root_p;
    begin_ms = get_current_ms();

    vector<Move> moves;
    addLegalMoves(p, moves);

    Move best_move = moves[0];
    Eval alpha = -INF_EVAL;

    Timestamp time_of_last_it = get_current_ms();
    BB last_it_nodes = nodes;

    for (int d = 1; d <= max_depth && alpha != INF_EVAL; d++) {
        for (Move &m : moves) {
            cout<<"currmove "<<m.getSAN()<<endl;
            p.makeMove(m);
            m.eval = -negaMax(p, d, -INF_EVAL, -alpha);
            p.undoMove();

            if (m.eval > alpha) {
                alpha = m.eval;
                best_move = m;

                if (alpha == INF_EVAL) break;
            }
        }
        cout<<"info ";
        cout<<"depth "<<to_string(d)<<" ";
        cout<<"score ";
        cout<<"cp "<<to_string(alpha)<<" ";
        cout<<"time "<<to_string(get_time_diff(begin_ms))<<" ";
        cout<<"nodes "<<to_string(nodes)<<" ";
        cout<<"nps "<<to_string((int)((double)(nodes-last_it_nodes)*1000/get_time_diff(time_of_last_it)))<<" ";
        sort(moves);
    }

    cout<<"bestmove "<<best_move.getSAN()<<endl;
}

void Search::stop() {
    searching = false;
}

/*
mutex mtx;
Task Search::getTask() {
    mtx.lock();
    Task p = tasks.front();
    tasks.pop();
    mtx.unlock();
    return p;
}
*/

Eval Search::negaMax(Pos &p, Depth depth, Eval alpha, Eval beta) {
    nodes++;
    if (!searching) return 0;
    if (depth == 0) return quies(p, alpha, beta);

    Eval original_alpha = alpha;

    bool found = false;
    TTEntry* entry = table.getEntry(p.key, found);
    Move entry_move = entry->move;
    if (found) {
        if (entry->depth >= depth) /*|| 
        ((entry->bound == LB || entry->bound == EXACT) && entry->eval >= beta) ||
        ((entry->bound == UB || entry->bound == EXACT) && entry->eval <= alpha))*/
            return entry->eval;
        
        /*
        if (entry->bound == LB && entry->eval >= alpha) alpha = max(alpha, entry->eval);
        else if (entry->bound == UB && entry->eval <= beta) beta = min(beta, entry->eval);*/
    }
    
    vector<Move> moves;
    addLegalMoves(p, moves);

    if (moves.size() == 0) return (p.inCheck ? -INF_EVAL : 0);

    order(moves, entry_move, Move());

    Move best_move = moves[0];

    for (Move &m : moves) {
        p.makeMove(m);
        Eval eval = -negaMax(p, depth-1, -beta, -alpha);
        p.undoMove();

        if (eval > alpha) {
            alpha = eval;
            best_move = m;

            if (alpha >= beta) break;
        }
    }

    Bound_Flag flag = LB;

    if (alpha <= original_alpha) flag = UB;
    else if (alpha < beta) flag = EXACT;
    else flag = LB;

    entry->save(p.key, best_move, alpha, depth, p.move_clock, flag);

    return alpha;
}

Eval Search::quies(Pos &p, Eval alpha, Eval beta) { //REMEMBER OPTIMIZE THIS
    nodes++;
    Eval stand_pat = evalPos(p, alpha, beta);
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;

    vector<Move> moves;
    addLegalMoves(p, moves);
    
    for (Move &m : moves) {
        if (m.isCapture() || m.isPromotion()) {
            p.makeMove(m);
            alpha = max(alpha, (Eval)-quies(p, -beta, -alpha));
            p.undoMove();
            if (alpha >= beta) {
                return beta;
            }
        }
    }
    return alpha;
}