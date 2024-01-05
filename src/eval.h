#pragma once

#include "types.h"
#include "pos.h"
#include <functional>

using namespace std;

extern vector<Eval> piece_eval;

const Eval STARTPOS_SUM_MAT_SQUARED = 17748;
const Eval WINNING_THRESHOLD = 900;

inline Eval get_piece_eval(Piece p) {
    return piece_eval[p];
}

inline Eval sum_mat_squared(Pos& pos, Color color) {
    return bitcount(pos.get_piece_mask(color, PAWN)) * 100
        + bitcount(pos.get_piece_mask(color, KNIGHT)) * 900
        + bitcount(pos.get_piece_mask(color, BISHOP)) * 1024
        + bitcount(pos.get_piece_mask(color, ROOK)) * 2500
        + bitcount(pos.get_piece_mask(color, QUEEN)) * 8100;
}

inline float get_early_weight(Pos& pos, Color color) {
    return ((float) pos.get_sum_mat_squared(opp(color))) / STARTPOS_SUM_MAT_SQUARED;
}

inline float get_late_weight(Pos& pos, Color color) {
    return 1 - get_early_weight(pos, color);
}

inline int sqMapTrans(int sq) { return rc(7-(sq/8), sq%8); }

inline int sqMapTrans(Color& color, int sq) { return color == WHITE ? sqMapTrans(sq) : sq; }

Eval eval_pos(Pos& p, Eval LB, Eval UB, bool debug = false);

struct Factor {
    string name;
    function<Eval(Pos&, Color)> func;

    Factor(string n, function<Eval(Pos&, Color)> f) {
        name = n;
        func = f;
    }
};