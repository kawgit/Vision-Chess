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
    root_p = p;
}

void Search::go() {
    searching = true;
    Pos p = root_p;
    begin_ms = get_current_ms();

    vector<Move> moves;
    addLegalMoves(p, moves);

    Move best_move = moves[0];
    Eval alpha = 0;

    Timestamp time_of_last_it = get_current_ms();
    BB last_it_nodes = nodes;

    order(moves, MOVENONE, MOVENONE);

    for (int d = 2; d <= max_depth && abs(alpha) <= INF_EVAL-200 && get_current_ms()-begin_ms < max_ms; d++) {
        alpha = -INF_EVAL;
        for (Move &m : moves) {
            cout<<"currmove "<<m.getSAN()<<" ";
            p.makeMove(m);
            m.eval = -negaMax(p, d-1, -INF_EVAL, INF_EVAL);
            p.undoMove();
            cout<<(int)m.eval<<endl;
            if (m.eval > alpha) {
                alpha = m.eval;
                best_move = m;

                if (alpha > INF_EVAL-200) break;
            }
        }
        sort(moves);
        bool found = false;


        table.getEntry(p.key, found)->save(p.key, best_move, alpha, d, EXACT);

        //UCI

        cout<<"info ";
        cout<<"depth "<<to_string(d)<<" ";
        cout<<"score ";
        cout<<"cp "<<to_string(alpha)<<" ";
        cout<<"time "<<to_string(get_time_diff(begin_ms))<<" ";
        cout<<"nodes "<<to_string(nodes)<<" ";
        cout<<"nps "<<to_string((int)((double)(nodes-last_it_nodes)*1000/get_time_diff(time_of_last_it)))<<" ";
        cout<<"hashfull "<<to_string(table.hashfull())<<" ";
        cout<<"pv ";

        vector<Move> pv = table.getPV(p);
        for (Move &m : pv) cout<<m.getSAN()<<" ";
        cout<<endl;
    }

    root_bestmove = best_move;
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

    //Eval mat_score = evalMat(p);
    //if (depth <= FPRUNE_DEPTH && mat_score + FPRUNE_MARGIN <= alpha) return quies(p, alpha, beta);
    //if (depth == RAZOR_DEPTH && mat_score + RAZOR_MARGIN <= alpha) depth--;

    Eval original_alpha = alpha;

    bool found = false;
    TTEntry* entry = table.getEntry(p.key, found);
    Move entry_move = entry->move;

    if (found) {
        if (entry->depth >= depth) {
            if ((entry->bound == LB && entry->eval >= beta) ||
                (entry->bound == UB && entry->eval <= alpha))
                return entry->eval;
        }
    }
    
    vector<Move> moves;
    addLegalMoves(p, moves);

    if (moves.size() == 0) return (p.inCheck ? -INF_EVAL + p.move_clock : 0);

    Move &prev_move = p.move_log.back();
    order(moves, entry_move, cm_hueristic[prev_move.getFr()][prev_move.getTo()]);

    Move best_move = moves[0];

    for (int i = 0; i < moves.size(); i++) {
        Move &m = moves[i];
        p.makeMove(m);
        Eval eval = -negaMax(p, (depth >= MIN_LMR_DEPTH && i >= LMR_MARGIN && !p.inCheck && !m.isCheck()) ? depth - 2 : depth-1, -beta, -alpha);
        p.undoMove();

        if (eval > alpha) {
            alpha = eval;
            best_move = m;

            if (alpha >= beta) {
                Move &prev_move = p.move_log.back();
                cm_hueristic[prev_move.getFr()][prev_move.getTo()] = m;
                break;
            }
        }
    }

    Bound_Flag flag = LB;

    if (alpha <= original_alpha) flag = UB;
    else if (alpha < beta) flag = EXACT;
    else flag = LB;


    if (entry->depth < depth) entry->save(p.key, best_move, alpha, depth, flag);

    return alpha;
}

Eval Search::quies(Pos &p, Eval alpha, Eval beta) { //REMEMBER OPTIMIZE THIS, ALSO SHOULD INCLUDE CHECKS
    nodes++;
    Eval stand_pat = evalPos(p, alpha, beta);
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;

    vector<Move> moves;
    addLegalMoves(p, moves);
    
    for (Move &m : moves) {
        if (m.isCapture() || m.isPromotion() || m.isCheck()) {
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