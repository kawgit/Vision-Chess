#include "eval.h"
#include "types.h"
#include "pos.h"
#include "util.h"

Eval mat_points[6] = {100, 320, 330, 500, 900, 0};

Eval piece_eval_maps[5][64] = {

    { //pawn
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    },

    { //knight
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    },

    { //bishop
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    },

    { //rook
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    },

    { //queen
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    }
};

Eval king_eval_map[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

Eval evalMat(Pos &p) {
    return (
        (bitcount(p.pieces[p.turn][PAWN]) - bitcount(p.pieces[p.notturn][PAWN])) * mat_points[PAWN] +
        (bitcount(p.pieces[p.turn][KNIGHT]) - bitcount(p.pieces[p.notturn][KNIGHT])) * mat_points[KNIGHT] +
        (bitcount(p.pieces[p.turn][BISHOP]) - bitcount(p.pieces[p.notturn][BISHOP])) * mat_points[BISHOP] +
        (bitcount(p.pieces[p.turn][ROOK]) - bitcount(p.pieces[p.notturn][ROOK])) * mat_points[ROOK] +
        (bitcount(p.pieces[p.turn][QUEEN]) - bitcount(p.pieces[p.notturn][QUEEN])) * mat_points[QUEEN]
    );
}

int sqMapTrans(int sq) {
    return RC2SQ(7-(sq/8), sq%8);
}

Eval evalPos(Pos &p, Eval LB, Eval UB) {
    Eval mat = evalMat(p);

    if (mat < LB - 100 || mat > UB + 100) return mat;

    Eval map = 0;

    for (int pt = PAWN; pt < KING; pt++) {
        {
            BB pieces = p.getPieceMask(p.turn, pt);
            while (pieces) {
                int sq = poplsb(pieces);
                map += piece_eval_maps[pt][p.turn == WHITE ? sqMapTrans(sq) : sq];
            }
        }

        {
            BB pieces = p.getPieceMask(p.notturn, pt);
            while (pieces) {
                int sq = poplsb(pieces);
                map -= piece_eval_maps[pt][p.notturn == WHITE ? sqMapTrans(sq) : sq];
            }
        }
    }

    return mat + map;
}