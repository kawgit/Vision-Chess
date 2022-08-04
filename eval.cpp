#include "eval.h"
#include "types.h"
#include "search.h"
#include "pos.h"
#include "bits.h"
#include "movegen.h"
#include "search.h"
#include <iostream>
#include <vector>
#include <functional>
#include <iomanip>

using namespace std;

vector<float> piece_eval = {100, 100, 100, 100, 100, 100};

vector<vector<float>> piece_eval_maps = {

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

vector<vector<float>> king_eval_map = {
    {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    60, 50, 20,  0,  0, 20, 50, 60
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

vector<Factor> factors = {
    Factor("material", [](Pos& pos, Color color) {
        return eval_mat(pos, color);
    }),

/*
    Factor("maps", [](Pos& pos, Color color) {
        Eval res = 0;

        for (int pt = PAWN; pt < KING; pt++) {
            BB pieces = pos.pieces(color, pt);
            while (pieces) {
                int sq = poplsb(pieces);
                res += piece_eval_maps[pt - PAWN][color == WHITE ? sqMapTrans(sq) : sq];
            }
        }

        return res;
    }),
    Factor("pawn_structure", [](Pos& pos, Color color) {

        const static Eval w1[8] = {0, 10, 20, 30, 70, 90, 120, 900};
        const static Eval w2[8] = {0, 40, 50, 60, 90, 120, 230, 900};
        const static Eval w3[8] = {0, 40, 50, 60, 90, 120, 340, 900};

        Eval res = 0;

        res -= bitcount(pos.isolated_pawns(color)) * 10;
        res -= bitcount(pos.doubled_pawns(color)) * 10;
        res -= bitcount(pos.blocked_pawns(color)) * 10;

        res += bitcount(pos.supported_pawns(color)) * 10;
        
        BB passed_pawns = pos.passed_pawns(color);

        if (passed_pawns) {

            BB supported_passed_pawns = passed_pawns & pos.supported_pawns(color);

            BB double_passed_pawns = passed_pawns & (files_of(passed_pawns) & files_of(shift<WEST>(passed_pawns)));

            while (passed_pawns) {
                Square sq = poplsb(passed_pawns);
                res += w1[rel_rank_of(color, sq)];
            }

            while (supported_passed_pawns) {
                Square sq = poplsb(supported_passed_pawns);
                res += w2[rel_rank_of(color, sq)];
            }

            while (double_passed_pawns) {
                Square sq = poplsb(double_passed_pawns);
                res += w3[rel_rank_of(color, sq)];
            }
        }

        return res;
    }),
*/
    Factor("space", [](Pos& pos, Color color) {
        Eval res = 0;

        res += bitcount(pos.get_atk_mask(color) & ~pos.pieces(color, PAWN)) * 4;

        return res;
    }),
/*
    Factor("king_safety", [](Pos& pos, Color color) {
        Eval res = 0;

        BB ksq_bb = pos.pieces(color, KING);
        
        int ksq = lsb(ksq_bb);

        BB pawns = pos.pieces(color, PAWN);
        BB surroundings = get_king_atk(ksq);

        res += bitcount(get_file_mask(ksq % 8) & pawns) * 20;
        res += bitcount(files_of(surroundings) & pawns) * 10;
        res += bitcount(files_of(surroundings) & pos.supported_pawns(color)) * 20;
        res += bitcount(surroundings & pawns & pos.get_atk_mask(color)) * 10;

        res -= bitcount(surroundings & pos.get_atk_mask(opp(color))) * 30;

        return res;
    }),
*/

};

Eval eval_pos(Pos& pos, Eval lb, Eval ub, bool debug) {
    Eval eval = 0;

    if (debug) print(pos, true);

    for (Factor factor : factors) {
        Eval ours = factor.func(pos, pos.turn);
        Eval theirs = factor.func(pos, pos.notturn);
        eval += ours;
        eval -= theirs;
        if (debug) cout << "factor: " << setw(15) << factor.name << " = (" << setw(8) << to_string(ours) << ") - (" << setw(8) << to_string(theirs) << ") = " << setw(8) << to_string(ours - theirs) << endl;
    }

    return eval;
}

Eval eval_mat(Pos& p, Color c) {
    return (
        bitcount(p.pieces(c, PAWN)) * piece_eval[PAWN - PAWN] +
        bitcount(p.pieces(c, KNIGHT)) * piece_eval[KNIGHT - PAWN] +
        bitcount(p.pieces(c, BISHOP)) * piece_eval[BISHOP - PAWN] +
        bitcount(p.pieces(c, ROOK)) * piece_eval[ROOK - PAWN] +
        bitcount(p.pieces(c, QUEEN)) * piece_eval[QUEEN - PAWN]
    );
}








