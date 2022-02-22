#include "eval.h"
#include "types.h"
#include "search.h"
#include "pos.h"
#include "bits.h"
#include "movegen.h"
#include "nnue.h"
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

	if (p.nnue != nullptr) return p.nnue->evaluate(p.turn);

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

    map += king_eval_map[totalmat < 1500][lsb(p.getPieceMask(p.turn, KING))] - king_eval_map[totalmat < 1500][lsb(p.getPieceMask(p.notturn, KING))];

    //PAWN EVALUATION
    Eval pawnEval = evalPawns(p, p.turn) - evalPawns(p, p.notturn);

    //KING SAFETY
    Eval kingSafety = evalKingSafety(p, p.turn) - evalKingSafety(p, p.notturn);

    //OUTPOST = GOOD

    return mat + map + pawnEval + kingSafety;
}

Eval evalMat(Pos& p, Color c) {
    return (
        bitcount(p.pieces[c][PAWN]) * mat_points[PAWN] +
        bitcount(p.pieces[c][KNIGHT]) * mat_points[KNIGHT] +
        bitcount(p.pieces[c][BISHOP]) * mat_points[BISHOP] +
        bitcount(p.pieces[c][ROOK]) * mat_points[ROOK] +
        bitcount(p.pieces[c][QUEEN]) * mat_points[QUEEN]
    );
}

Eval evalPawns(Pos& p, Color c) {
	Eval eval = 0;

    //double pawns = bad
    for (int i = 0; i < 8; i++) {
        BB pawns = p.getPieceMask(c, PAWN) & getColMask(i);
        if (pawns) eval -= (bitcount(pawns) - 1) * 40;
	}

    //supported pawns = good
	BB pawns = p.getPieceMask(c, PAWN);
	if (pawns) eval += (bitcount(((pawns & ~getColMask(7)) << 9) & pawns) + bitcount(((pawns & ~getColMask(0)) << 7) & pawns)) * 5;

    //isolated pawns = bad
    //passed pawns = good

	return eval;
}

Eval evalKingSafety(Pos& p, Color c) {
	Eval eval = 0;
	Square ksq = lsb(p.getPieceMask(c, KING));
	eval += bitcount(getKingAtk(ksq) & p.getPieceMask(c, PAWN)) * 15;
	BB occ = p.getOcc(c);
	eval -= bitcount(getBishopAtk(ksq, occ) & ~occ) * 12;
	eval -= bitcount(getColMask(ksq%8) & getRookAtk(ksq, p.getOcc(c))) * 10;
	return eval;
}











