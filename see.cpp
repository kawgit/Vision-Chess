/*
#include "pos.h"
#include "types.h"
#include "eval.h"
#include "see.h"
#include "movegen.h"
#include "bits.h"

using namespace std;

Eval see_search(Pos& pos, Move move) {
    
    Square to_sq = get_to(move);
    Square from_sq = get_from(move);
    Color us = pos.color_at(get_from(move));
    Color them = opp(us);

    BB us_occ = 0;
    us_occ |= (pos.pieces(us, PAWN)) & get_pawn_atk(them, to_sq);
    us_occ |= (pos.pieces(us, KNIGHT)) & get_knight_atk(to_sq);
    us_occ |= (pos.pieces(us, KING)) & get_king_atk(to_sq);
    us_occ |= (pos.pieces(us, BISHOP) | pos.pieces(us, QUEEN)) & get_bishop_atk(to_sq, 0);
    us_occ |= (pos.pieces(us, ROOK) | pos.pieces(us, QUEEN)) & get_rook_atk(to_sq, 0);
    us_occ &= ~get_BB(from_sq);

    BB them_occ = 0;
    them_occ |= (pos.pieces(them, PAWN)) & get_pawn_atk(us, to_sq);
    them_occ |= (pos.pieces(them, KNIGHT)) & get_knight_atk(to_sq);
    them_occ |= (pos.pieces(them, KING)) & get_king_atk(to_sq);
    them_occ |= (pos.pieces(them, BISHOP) | pos.pieces(them, QUEEN)) & get_bishop_atk(to_sq, 0);
    them_occ |= (pos.pieces(them, ROOK) | pos.pieces(them, QUEEN)) & get_rook_atk(to_sq, 0);
    them_occ &= ~get_BB(from_sq);

    Eval eval_on_square = get_piece_eval(pos.mailboxes(them, to_sq));
    Eval eval_gain = 0;
    Color turn = us;
    
    eval_gain += eval_on_square;
    eval_on_square = get_piece_eval(pos.mailboxes(us, from_sq));
    turn = opp(us);

    BB occ = pos.get_occ();
    int nums[] = {0, 0, 0, 0, 0, 0};

    for (Piece pt = PAWN; pt <= KING; pt) {
        BB us_atkers = pos.pieces(us, pt) & get_piece_atk(pt, to_sq, us, occ);
        BB them_atkers = pos.pieces(them, pt) & get_piece_atk(pt, to_sq, them, occ);
    
        nums[pt - PAWN] = bitcount(us_atkers) - bitcount(them_atkers);

        occ &= ~us_atkers;
        occ &= ~them_atkers;
    }
}

1 1
p n b r q k
0 1
0 1
+1

1 1
p n b r q k
0 0 0 0 0
0 0 0 0 1
0

1 1
p n b r q k
0 1 0 0 0 0
1 0 0 0 1

1 1
p n b r q k
0 1 0 0 0 0
0 0 0 1 1 0
+1


1 1
p n b r q k
1 0 0 0 0 0
0 0 0 1 1 0
+1
*/