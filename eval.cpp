#include "eval.h"
#include "types.h"
#include "search.h"
#include "pos.h"
#include "bits.h"
#include "movegen.h"
#include "search.h"
#include <iostream>
#include <vector>

using namespace std;

Eval mat_points[6] = {100, 320, 330, 500, 900, 20000};

Eval piece_eval_maps[5][64] = {

    { //pawn
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-30,-30, 10, 10,  5,
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
        5, 15, 15, 15, 15, 15, 15,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  8,  8,  0,  0,  0
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

Eval king_eval_map[2][64] = {
    {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
    },  

    {
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

int sqMapTrans(int sq) {
    return rc(7-(sq/8), sq%8);
}

Eval evalPos(Pos& p, Eval lb, Eval ub) {
    Eval turnmat = evalMat(p, p.turn);
    Eval notturnmat = evalMat(p, p.notturn);
    Eval mat = turnmat - notturnmat;
    Eval totalmat = turnmat + notturnmat;

    if (mat < lb - 100 || mat > ub + 100) return mat;

    Eval map = 0;

    for (int pt = PAWN; pt < KING; pt++) {

        BB our_pieces = p.pieces(p.turn, pt);
        while (our_pieces) {
            int sq = poplsb(our_pieces);
            map += piece_eval_maps[pt - PAWN][p.turn == WHITE ? sqMapTrans(sq) : sq];
        }

        BB their_pieces = p.pieces(p.notturn, pt);
        while (their_pieces) {
            int sq = poplsb(their_pieces);
            map -= piece_eval_maps[pt - PAWN][p.notturn == WHITE ? sqMapTrans(sq) : sq];
        }
    }
    
    return mat + map;
}

Eval evalMat(Pos& p, Color c) {
    return (
        bitcount(p.pieces(c, PAWN)) * mat_points[PAWN - PAWN] +
        bitcount(p.pieces(c, KNIGHT)) * mat_points[KNIGHT - PAWN] +
        bitcount(p.pieces(c, BISHOP)) * mat_points[BISHOP - PAWN] +
        bitcount(p.pieces(c, ROOK)) * mat_points[ROOK - PAWN] +
        bitcount(p.pieces(c, QUEEN)) * mat_points[QUEEN - PAWN]
    );
}








