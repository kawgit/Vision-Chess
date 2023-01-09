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

// const vector<const float> connectivity_victim_weights = {50, 35, 30, 10, 4, 0};

// const vector<const float> connectivity_weights = {};

vector<float> piece_eval = {100, 300, 320, 500, 900, 100};

vector<vector<float>> piece_eval_maps = {

    { //pawn
         0,  0,  0,  0,  0,  0,  0,  0,
        20, 20, 20, 20, 20, 20, 20, 20,
        10, 10, 20, 30, 30, 20, 10, 10,
         5,  5, 10, 20, 20, 10,  5,  5,
         0,  0,  0, 10, 10,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,-10,-10,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0
    },

    { //knight
        -50,-40,-20,-20,-20,-20,-20, -50,
        -40,-20,  0,  0,  0,  0,-20, -40,
        -30,  0, 10, 15, 15, 10,  0, -30,
        -30,  0, 15, 20, 20, 15,  0, -30,
        -30,  0, 15, 20, 20, 15,  0, -30,
        -30,  0, 10, 15, 15, 10,  0, -30,
        -40,-20,  0,  5,  5,  0,-20, -40,
        -50,-40,-20,-20,-20,-20,-20, -50,
    },

    { //bishop
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
    },

    { //rook
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
    },

    { //queen
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
          0,  0,  0,  0,  0,  0,  0,  0,
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
         10, 10,-10,-10,-10,-10, 10, 10,
         20, 20, 10,  0,  0, 10, 20, 20
    },  

    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10, -5,  0,  0,  0,  0, -5,-10,
        -10,  0, 15, 15, 15, 15,  0,-10,
        -5,   0, 15, 15, 15, 15,  0, -5,
        -5,   0, 15, 15, 15, 15,  0, -5,
        -10,  0, 15, 15, 15, 15,  0,-10,
        -10, -5,  0,  0,  0,  0, -5,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    }
};

vector<Factor> factors = {
    Factor("material", [](Pos& pos, Color color) {
        return eval_mat(pos, color);
    }),

    Factor("maps", [](Pos& pos, Color color) {
        Eval res = 0;

        for (int pt = PAWN; pt < KING; pt++) {
            BB pieces = pos.pieces(color, pt);
            while (pieces) {
                int sq = poplsb(pieces);
                res += piece_eval_maps[pt - PAWN][color == WHITE ? sqMapTrans(sq) : sq];
            }
        }

        Square ksq = lsb(pos.pieces(color, KING));
        bool is_endgame = !(pos.pieces(opp(color), QUEEN) ||
            bitcount(pos.pieces(opp(color), ROOK)) >= 2 ||
            bitcount(pos.pieces(opp(color), KNIGHT)) + bitcount(pos.pieces(opp(color), BISHOP)) + bitcount(pos.pieces(opp(color), ROOK)) >= 3);
        
        res += king_eval_map[is_endgame ? 1 : 0][color == WHITE ? sqMapTrans(ksq) : ksq];

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

    Factor("control", [](Pos& pos, Color color) {
        Eval res = 0;
        char cum[64] = {};

		add_bb_to_cum(cum, shift<NORTH>(shift<WEST>(pos.pieces(color, PAWN)) | shift<EAST>(pos.pieces(color, PAWN))), 50);

        {
            BB knights = pos.pieces(color, KNIGHT);
            while (knights) {
                int from = poplsb(knights);
                add_bb_to_cum(cum, get_knight_atk(from), 30);
            }
        }

        {
            BB occ = pos.get_occ() & ~(pos.pieces(opp(color), KING) | pos.pieces(color, BISHOP) | pos.pieces(color, QUEEN));
            BB bishops = pos.pieces(color, BISHOP) | pos.pieces(color, QUEEN);
            while (bishops) {
                int from = poplsb(bishops);
                add_bb_to_cum(cum, get_bishop_atk(from, occ), 25);
            }
        }

        {
            BB occ = pos.get_occ() & ~(pos.pieces(opp(color), KING) | pos.pieces(color, ROOK) | pos.pieces(color, QUEEN));
            BB rooks = pos.pieces(color, ROOK) | pos.pieces(color, QUEEN);
            while (rooks) {
                int from = poplsb(rooks);
                add_bb_to_cum(cum, get_rook_atk(from, occ), 15);
            }
        }

        for (Square sq = 0; sq < 64; sq++) {
            float val = ((float)(cum[sq]) / 10);
            if (val >= 0) {
                res += 2;
            }
        }

        return res;
    }),

    Factor("mobility", [](Pos& pos, Color color) {
        Eval res = 0;
        {
            BB knights = pos.pieces(color, KNIGHT);
            while (knights) {
                int from = poplsb(knights);
                res += bitcount(get_knight_atk(from));
            }
        }

        {
            BB occ = pos.get_occ() & ~(pos.pieces(opp(color), KING) | pos.pieces(color, BISHOP) | pos.pieces(color, QUEEN));
            BB bishops = pos.pieces(color, BISHOP) | pos.pieces(color, QUEEN);
            while (bishops) {
                int from = poplsb(bishops);
                res += bitcount(get_bishop_atk(from, occ)) * 3;
            }
        }

        {
            BB occ = pos.get_occ() & ~(pos.pieces(opp(color), KING) | pos.pieces(color, ROOK) | pos.pieces(color, QUEEN));
            BB rooks = pos.pieces(color, ROOK) | pos.pieces(color, QUEEN);
            while (rooks) {
                int from = poplsb(rooks);
                res += bitcount(get_rook_atk(from, occ)) * 5;
            }
        }
        return res;
    }),

    Factor("king_safety", [](Pos& pos, Color color) {
        Eval res = 0;

        BB ksq_bb = pos.pieces(color, KING);
        int ksq = lsb(ksq_bb);

        BB surr = get_king_atk(ksq);
        //res += (8 - bitcount(surr)) * 4;
        res += bitcount(surr & pos.pieces(color, PAWN)) * 9;
        res += bitcount(surr & pos.pieces(color, KNIGHT)) * 1;
        res += bitcount(surr & pos.pieces(color, BISHOP)) * 2;
        
        BB temp = surr;
        while (temp) {
            Square square = poplsb(temp);
            if (pos.get_control_value(color, square) < 0) {
                res -= 10;
            }
        }

        res -= bitcount(pos.get_atk_mask(opp(color)) & surr) * 8;

        return res;
    }),


    /*Factor("connectivity", [](Pos& pos, Color color) {
        Eval res = 0;

        for (Piece victim_type = PAWN; victim_type <= KNIGHT; victim_type++) {
            
            BB victims = pos.pieces(color, victim_type);
            while (victims) {
                Square victim_square = poplsb(victims);
                for (Piece defender_type = PAWN; defender_type < KING; defender_type++) {
                    defender_type 
                }
            }
        }

        return res;
    }),*/

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








