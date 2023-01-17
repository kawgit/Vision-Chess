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

vector<Eval> piece_eval = {0, 100, 300, 320, 500, 900, 10000};

vector<Eval> early_pawn_map = {
      0,  0,  0,  0,  0,  0,  0,  0,
     20, 20, 20, 20, 20, 20, 20, 20,
     10, 10, 20, 30, 30, 20, 10, 10,
      5,  5, 10, 20, 20, 10,  5,  5,
      0,  0,  0, 10, 10,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,-10,-10,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0
};

vector<Eval> anytime_knight_map = {
    -40,-30,-20,-20,-20,-20,-30, -40,
    -30,-10,  0,  0,  0,  0,-10, -30,
    -15,  0, 10, 15, 15, 10,  0, -15,
    -15,  0, 15, 20, 20, 15,  0, -15,
    -15,  0, 15, 20, 20, 15,  0, -15,
    -15,  0, 10, 15, 15, 10,  0, -15,
    -30,-10,  0,  5,  5,  0,-10, -30,
    -40,-30,-20,-20,-20,-20,-30, -40,
};

vector<Eval> early_bishop_map = {
      0,  0,  0,  0,  0,  0,  0,  0,
      0, 10, 10,  0,  0, 10, 10,  0,
      0, 10, 10, 10, 10, 10, 10,  0,
      0,  0, 10, 10, 10, 10,  0,  0,
      0,  0, 20, 10, 10, 20,  0,  0,
      0, 10, 10, 10, 10, 10, 10,  0,
      0, 20, 10,  0, 10, 10, 20,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
};

vector<Eval> early_king_map = {
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 20,  0,  0,  0,  0, 20, 20,
};

vector<Eval> late_king_map = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10, -5,  0,  0,  0,  0, -5,-10,
    -10,  0, 15, 15, 15, 15,  0,-10,
    -5,   0, 15, 15, 15, 15,  0, -5,
    -5,   0, 15, 15, 15, 15,  0, -5,
    -10,  0, 15, 15, 15, 15,  0,-10,
    -10, -5,  0,  0,  0,  0, -5,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

vector<Factor> lazy_factors = {
    Factor("material", [](Pos& pos, Color color) {
        return pos.ref_mat(color);
    }),
    
    Factor("pawn_structure", [](Pos& pos, Color color) {

        const static Eval w1[8] = {0, 20, 30, 40, 50,  90, 120, 900};
        const static Eval w2[8] = {0, 40, 50, 70, 120, 220, 340, 900};

        Eval res = 0;

        res -= bitcount(pos.isolated_pawns(color)) * 5;
        res -= bitcount(pos.doubled_pawns(color)) * 10;
        res -= bitcount(pos.blocked_pawns(color)) * 2;

        res += bitcount(pos.supported_pawns(color)) * 3;
        
        BB passed_pawns = pos.passed_pawns(color);

        if (passed_pawns) {

            BB double_passed_pawns = passed_pawns & (files_of(passed_pawns) & files_of(shift<WEST>(passed_pawns)));
            passed_pawns &= ~double_passed_pawns;

            while (passed_pawns) {
                Square sq = poplsb(passed_pawns);
                res += w1[rel_rank_of(color, sq)];
            }

            while (double_passed_pawns) {
                Square sq = poplsb(double_passed_pawns);
                res += w2[rel_rank_of(color, sq)];
            }
        }

        return res;
    }),

    Factor("king_safety", [](Pos& pos, Color color) {
        Eval res = 0;

        BB ksq_bb = pos.ref_piece_mask(color, KING);
        int ksq = lsb(ksq_bb);

        BB surr = get_king_atk(ksq);
        //res += (8 - bitcount(surr)) * 4;
        res += bitcount(surr & pos.ref_piece_mask(color, PAWN)) * 9;
        res += bitcount(surr & pos.ref_piece_mask(color, KNIGHT)) * 1;
        res += bitcount(surr & pos.ref_piece_mask(color, BISHOP)) * 2;
        
        BB temp = surr;
        while (temp) {
            Square square = poplsb(temp);
            if ((pos.ref_atk(opp(color)) & get_BB(square)) && pos.get_control_value(color, square) < 0) {
                res -= 10;
            }
        }

        res -= bitcount(pos.ref_atk(opp(color)) & surr) * 8;

        return res;
    }),
    
    Factor("early_king", [](Pos& pos, Color color) {
        return (Eval)(get_early_weight(pos, color) * early_king_map[sqMapTrans(color, lsb(pos.ref_piece_mask(color, KING)))]);
    }),

    Factor("late_king", [](Pos& pos, Color color) {
        return (Eval)(get_late_weight(pos, color) * late_king_map[sqMapTrans(color, lsb(pos.ref_piece_mask(color, KING)))]);
    }),

    Factor("early_bishop", [](Pos& pos, Color color) {
        Eval result = 0;
        BB pieces = pos.ref_piece_mask(color, BISHOP);
        while (pieces) {
            Square from = poplsb(pieces);
            result += early_bishop_map[sqMapTrans(color, from)];
        }
        return (Eval)(get_early_weight(pos, color) * result);
    }),

    Factor("early_pawn", [](Pos& pos, Color color) {
        Eval result = 0;
        BB pieces = pos.ref_piece_mask(color, PAWN);
        while (pieces) {
            Square from = poplsb(pieces);
            result += early_pawn_map[sqMapTrans(color, from)];
        }
        return (Eval)(get_early_weight(pos, color) * result);
    }),

    Factor("anytime_knight", [](Pos& pos, Color color) {
        Eval result = 0;
        BB pieces = pos.ref_piece_mask(color, KNIGHT);
        while (pieces) {
            Square from = poplsb(pieces);
            result += anytime_knight_map[sqMapTrans(color, from)];
        }
        return result;
    }),
};

vector<Factor> tough_factors = {
    /*Factor("mobility", [](Pos& pos, Color color) {
        Eval res = 0;
        BB us = pos.ref_occ(color);
        BB occ = pos.ref_occ(opp(color)) | us;

        for (Piece pt = BISHOP; pt <= QUEEN; pt++) {
            BB froms = pos.ref_piece_mask(color, pt);
            while (froms) {
                int from = poplsb(froms);
                BB tos = get_piece_atk(pt, from, color, occ) & ~us;
                Eval max_gain = -INF;
                while (tos) {
                    int to = poplsb(tos);
                    int gain = sea_gain(pos, make_move(from, to, (get_BB(to) & occ) ? CAPTURE : QUIET), -INF);
                    max_gain = max(max_gain, gain);
                    if (gain >= 0) break;
                }

                if (max_gain < 0) {
                    res += max(-get_piece_eval(pt), max_gain);
                }
            }
        }

        return res / 10;
    }),*/

    /*Factor("connectivity", [](Pos& pos, Color color) {
        Eval res = 0;

        for (Piece victim_type = PAWN; victim_type <= KNIGHT; victim_type++) {
            
            BB victims = pos.ref_piece_mask(color, victim_type);
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

    if (debug) {
        cout << "white perspective early weight: " << to_string(get_early_weight(pos, WHITE)) << endl;
        cout << "black perspective early weight: " << to_string(get_early_weight(pos, BLACK)) << endl;
    }

    for (Factor factor : lazy_factors) {
        Eval ours = factor.func(pos, pos.turn);
        Eval theirs = factor.func(pos, pos.notturn);
        eval += ours;
        eval -= theirs;
        if (debug) cout << "factor: " << setw(15) << factor.name << " = (" << setw(8) << to_string(ours) << ") - (" << setw(8) << to_string(theirs) << ") = " << setw(8) << to_string(ours - theirs) << endl;
    }

    if (eval < lb - 100 || eval > ub + 100) {
        return eval;
    }

    for (Factor factor : tough_factors) {
        Eval ours = factor.func(pos, pos.turn);
        Eval theirs = factor.func(pos, pos.notturn);
        eval += ours;
        eval -= theirs;
        if (debug) cout << "factor: " << setw(15) << factor.name << " = (" << setw(8) << to_string(ours) << ") - (" << setw(8) << to_string(theirs) << ") = " << setw(8) << to_string(ours - theirs) << endl;
    }

    return eval;
}








