#include "pos.h"
#include "types.h"
#include "bits.h"
#include "movegen.h"
#include "move.h"
#include <vector>
#include <cassert>

vector<Move> get_legal_moves(Pos& pos) {
    assert(pos.get_piece_mask(WHITE, KING) != 0);
    assert(pos.get_piece_mask(BLACK, KING) != 0);

	pos.update_atks();
	pos.update_pins_and_checks();
    if (!pos.slice->has_updated_atks) pos.update_atks();

    vector<Move> moves;
    moves.reserve(Pos::MOVES_RESERVE_SIZE);

    if (pos.get_num_checks() != 2) {
        add_pawn_moves(moves, pos);
        add_knight_moves(moves, pos);
        add_bishop_moves(moves, pos);
        add_rook_moves(moves, pos);
        add_queen_moves(moves, pos);
    }
    add_king_moves(moves, pos);
    
    return moves;
}

inline void add_pawn_moves_from_mask(vector<Move> &moves, Pos& pos, BB &mask, int transform, MoveFlag flags) {
    while (mask) {
        int to = poplsb(mask);
        int from = to - transform;
        if (pos.is_moveable(from, to))
            moves.push_back(make_move(from, to, flags));
    }
}

inline void add_pawn_ep_moves_from_mask(vector<Move> &moves, Pos& pos, BB &mask, int transform) {
    while (mask) {
        int to = poplsb(mask);
        int from = to - transform;
        if (pos.get_moveable_squares(from) & bb_of(to))
            moves.push_back(make_move(from, to, EP));
    }
}

inline void add_pawn_promotion_moves_from_mask(vector<Move> &moves, Pos& pos, BB &mask, int transform, MoveFlag flags) {
    while (mask) {
        int to = poplsb(mask);
        int from = to - transform;
        if (pos.is_moveable(from, to))
            for (MoveFlag prom_flag = N_PROM; prom_flag <= Q_PROM; prom_flag++)
                moves.push_back(make_move(from, to, flags | prom_flag));
    }
}

void add_pawn_moves(vector<Move>& moves, Pos& pos) {
    if (!pos.get_piece_mask(pos.turn, PAWN)) return;

    BB pawns = pos.get_piece_mask(pos.turn, PAWN);

    BB p1;
    BB p2;
    BB rc;
    BB lc;
    BB rc_ep = 0ULL;
    BB lc_ep = 0ULL;
    
    BB p_p1;
    BB p_rc;
    BB p_lc;

    int p1_t;
    int rc_t;
    int lc_t;

    if (pos.turn == WHITE) {
        p1_t = 8;
        rc_t = 9;
        lc_t = 7;

        p1 = (pawns << 8) & ~pos.get_occ();
        p2 = ((p1 & bb_of_rank(2)) << 8) & ~pos.get_occ();
        rc = ((pawns & ~bb_of_file(7)) << 9);
        lc = ((pawns & ~bb_of_file(0)) << 7);
        if (pos.get_ep() != SQUARE_NONE) {
            rc_ep = rc & bb_of(pos.get_ep());
            lc_ep = lc & bb_of(pos.get_ep());
        }
        rc &= pos.get_occ(pos.notturn);
        lc &= pos.get_occ(pos.notturn);

        p_p1 = p1 & bb_of_rank(7);
        p_rc = rc & bb_of_rank(7);
        p_lc = lc & bb_of_rank(7);

        p1 &= ~bb_of_rank(7);
        rc &= ~bb_of_rank(7);
        lc &= ~bb_of_rank(7);
    }
    else {
        p1_t = -8;
        rc_t = -9;
        lc_t = -7;

        p1 = (pawns >> 8) & ~pos.get_occ();
        p2 = ((p1 & bb_of_rank(5)) >> 8) & ~pos.get_occ();
        rc = ((pawns & ~bb_of_file(0)) >> 9);
        lc = ((pawns & ~bb_of_file(7)) >> 7);
        if (pos.get_ep() != SQUARE_NONE) {
            rc_ep = rc & bb_of(pos.get_ep());
            lc_ep = lc & bb_of(pos.get_ep());
        }
        rc &= pos.get_occ(pos.notturn);
        lc &= pos.get_occ(pos.notturn);
        
        p_p1 = p1 & bb_of_rank(0);
        p_rc = rc & bb_of_rank(0);
        p_lc = lc & bb_of_rank(0);

        p1 &= ~bb_of_rank(0);
        rc &= ~bb_of_rank(0);
        lc &= ~bb_of_rank(0);
    }

    add_pawn_moves_from_mask(moves, pos, p1, p1_t, QUIET);
    add_pawn_moves_from_mask(moves, pos, p2, p1_t + p1_t, DOUBLE_PAWN_PUSH);
    add_pawn_moves_from_mask(moves, pos, rc, rc_t, CAPTURE);
    add_pawn_moves_from_mask(moves, pos, lc, lc_t, CAPTURE);
    add_pawn_ep_moves_from_mask(moves, pos, rc_ep, rc_t);
    add_pawn_ep_moves_from_mask(moves, pos, lc_ep, lc_t);

    add_pawn_promotion_moves_from_mask(moves, pos, p_p1, p1_t, QUIET);
    add_pawn_promotion_moves_from_mask(moves, pos, p_rc, rc_t, CAPTURE);
    add_pawn_promotion_moves_from_mask(moves, pos, p_lc, lc_t, CAPTURE);
}

void add_knight_moves(vector<Move>& moves, Pos& pos) {
    BB knights = pos.get_piece_mask(pos.turn, KNIGHT);

    while (knights) {
        int from = poplsb(knights);
        BB atk = attacks::knight(from) & ~pos.get_occ(pos.turn);
        BB cap = atk & pos.get_occ(pos.notturn);
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (pos.is_moveable(from, to)) moves.push_back(make_move(from, to, CAPTURE));
        }
        while (qui) {
            int to = poplsb(qui);
            if (pos.is_moveable(from, to)) moves.push_back(make_move(from, to, QUIET));
        }
    }
}

void add_bishop_moves(vector<Move>& moves, Pos& pos) {
    BB bishop = pos.get_piece_mask(pos.turn, BISHOP);

    while (bishop) {
        int from = poplsb(bishop);
        BB atk = attacks::bishop(from, pos.get_occ()) & ~pos.get_occ(pos.turn);
        BB cap = atk & pos.get_occ(pos.notturn);
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (pos.is_moveable(from, to)) moves.push_back(make_move(from, to, CAPTURE));
        }
        while (qui) {
            int to = poplsb(qui);
            if (pos.is_moveable(from, to)) moves.push_back(make_move(from, to, QUIET));
        }
    }
}

void add_rook_moves(vector<Move>& moves, Pos& pos) {
    BB rooks = pos.get_piece_mask(pos.turn, ROOK);

    while (rooks) {
        int from = poplsb(rooks);
        BB atk = attacks::rook(from, pos.get_occ()) & ~pos.get_occ(pos.turn);
        BB cap = atk & pos.get_occ(pos.notturn);
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (pos.is_moveable(from, to)) moves.push_back(make_move(from, to, CAPTURE));
        }
        while (qui) {
            int to = poplsb(qui);
            if (pos.is_moveable(from, to)) moves.push_back(make_move(from, to, QUIET));
        }
    }
}

void add_queen_moves(vector<Move>& moves, Pos& pos) {
    BB queens = pos.get_piece_mask(pos.turn, QUEEN);

    while (queens) {
        int from = poplsb(queens);
        BB atk = attacks::queen(from, pos.get_occ()) & ~pos.get_occ(pos.turn);
        BB cap = atk & pos.get_occ(pos.notturn);
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (pos.is_moveable(from, to)) moves.push_back(make_move(from, to, CAPTURE));
        }
        while (qui) {
            int to = poplsb(qui);
            if (pos.is_moveable(from, to)) moves.push_back(make_move(from, to, QUIET));
        }
    }
}

const BB WKS_CLEARANCE = bb_of(F1) | bb_of(G1);
const BB WQS_CLEARANCE = bb_of(B1) | bb_of(C1) | bb_of(D1);

const BB BKS_CLEARANCE = bb_of(F8) | bb_of(G8);
const BB BQS_CLEARANCE = bb_of(B8) | bb_of(C8) | bb_of(D8);

const BB WKS_SAFE = bb_of(F1) | bb_of(G1);
const BB WQS_SAFE = bb_of(C1) | bb_of(D1);

const BB BKS_SAFE = bb_of(F8) | bb_of(G8);
const BB BQS_SAFE = bb_of(C8) | bb_of(D8);

void add_king_moves(vector<Move>& moves, Pos& pos) {
    int ksq = lsb(pos.get_piece_mask(pos.turn, KING));
    BB atk = attacks::king(ksq) & ~pos.get_occ(pos.turn);

    if (atk == 0) return;

    BB notturn_atk = pos.get_atk(pos.notturn);
    atk &= ~notturn_atk;

    BB cap = atk & pos.get_occ(pos.notturn);
    BB qui = atk & ~cap;

    while (cap) {
        int to = poplsb(cap);
        moves.push_back(make_move(ksq, to, CAPTURE));
    }
    while (qui) {
        int to = poplsb(qui);
        moves.push_back(make_move(ksq, to, QUIET));
    }

    //CASTLING
    if (pos.get_num_checks() == 0 && pos.get_cr()) {
        if (pos.turn == WHITE && (pos.get_cr() &  0b0011)) {
            if (getWK(pos.get_cr()) && !(pos.get_occ() & WKS_CLEARANCE) && !(notturn_atk & WKS_SAFE)) moves.push_back(make_move(ksq, G1, KING_CASTLE));
            if (getWQ(pos.get_cr()) && !(pos.get_occ() & WQS_CLEARANCE) && !(notturn_atk & WQS_SAFE)) moves.push_back(make_move(ksq, C1, QUEEN_CASTLE));
        }
        else if (pos.turn == BLACK && (pos.get_cr() &  0b1100)){
            if (getBK(pos.get_cr()) && !(pos.get_occ() & BKS_CLEARANCE) && !(notturn_atk & BKS_SAFE)) moves.push_back(make_move(ksq, G8, KING_CASTLE));
            if (getBQ(pos.get_cr()) && !(pos.get_occ() & BQS_CLEARANCE) && !(notturn_atk & BQS_SAFE)) moves.push_back(make_move(ksq, C8, QUEEN_CASTLE));
        }
    }
}
