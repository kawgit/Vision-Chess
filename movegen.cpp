#include "pos.h"
#include "types.h"
#include "bits.h"
#include "movegen.h"
#include <vector>
#include <cassert>

BB bishop_magics[64];
BB rook_magics[64];
BB bishop_table[64][1<<BISHOP_BITS];
BB rook_table[64][1<<ROOK_BITS];

BB rays[64][8];

BB rook_atks[64];
BB bishop_atks[64];

BB rook_blockermasks[64];
BB bishop_blockermasks[64];

BB pawn_atks[2][64];
BB knight_atks[64];
BB king_atks[64];

int directions[8][2] = {{0, 1},{1, 1},{1, 0},{1, -1},{0, -1},{-1, -1},{-1, 0},{-1, 1}};

BB getAnswer(int r, int c, BB blockerboard, int d_offset) {
    BB answer = 0;
    for (int d = d_offset; d < 8; d+=2) {
        for (int m = 1; m <= 7; m++) {
            int _r = r + directions[d][1]*m;
            int _c = c + directions[d][0]*m;
            int _s = rc(_r, _c);
            if (_r >= 0 && _r <= 7 && _c >= 0 && _c <= 7) {
                answer |= get_BB(_s);
                if (bitAt(blockerboard, _s)) break;
            }
            else break;
        }
    }
    return answer;
}

void init_movegen(int seed) {

    //generate rays and moves

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int s = rc(r, c);
            for (int d = 0; d < 8; d++) {
                for (int m = 1; m <= 7; m++) {
                    int _r = r + directions[d][1]*m;
                    int _c = c + directions[d][0]*m;
                    if (_r >= 0 && _r <= 7 && _c >= 0 && _c <= 7) {
                        rays[s][d] |= get_BB(rc(_r, _c));
                        if (m == 1) king_atks[s] |= get_BB(rc(_r, _c));
                    }
                }
            }
        }
    }

    for (int s = 0; s < 64; s++) {
        rook_atks[s] = rays[s][NORTH] | rays[s][EAST] | rays[s][SOUTH] | rays[s][WEST];
        bishop_atks[s] = rays[s][NORTHEAST] | rays[s][SOUTHEAST] | rays[s][SOUTHWEST] | rays[s][NORTHWEST];
        get_pawn_atk(WHITE, s) = (rays[s][NORTHEAST] | rays[s][NORTHWEST]) & king_atks[s];
        get_pawn_atk(BLACK, s) = (rays[s][SOUTHEAST] | rays[s][SOUTHWEST]) & king_atks[s];
    }

    int knight_directions[8][2] = {{1, 2},{2, 1},{2, -1},{1, -2},{-1, -2},{-2, -1},{-2, 1},{-1, 2}};
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int s = rc(r, c);
            for (int d = 0; d < 8; d++) {
                int _r = r + knight_directions[d][1];
                int _c = c + knight_directions[d][0];

                if (_r >= 0 && _r <= 7 && _c >= 0 && _c <= 7) 
                    knight_atks[s] |= get_BB(rc(_r, _c));
            }
        }
    }

    //generate blockermasks

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            BB exclude = 0ULL;
            if (c != 0) exclude |= get_file_mask(0);
            if (c != 7) exclude |= get_file_mask(7);
            if (r != 0) exclude |= get_rank_mask(0);
            if (r != 7) exclude |= get_rank_mask(7);

            exclude = 0ULL;
            int s = rc(r, c);
            rook_blockermasks[s] = rook_atks[s] & ~exclude;
            bishop_blockermasks[s] = bishop_atks[s] & ~exclude;
        }
    }

    //generate blockerboards

    vector<BB> rook_blockerboards[64];
    vector<BB> bishop_blockerboards[64];
    for (int s = 0; s < 64; s++) {
        {
            int count = bitcount(rook_blockermasks[s]);
            for (BB bits = 0; bits < (1ULL<<count); bits++) {
                BB save = rook_blockermasks[s];
                rook_blockerboards[s].push_back(rook_blockermasks[s]);
                for (int i = 0; i < count; i++) {
                    if (!bitAt(bits, i)) rook_blockerboards[s][bits] &= ~get_BB(poplsb(save));
                    else poplsb(save);
                }
            }
        }

        {        
            int count = bitcount(bishop_blockermasks[s]);
            for (BB bits = 0; bits < (1ULL<<count); bits++) {
                BB save = bishop_blockermasks[s];
                bishop_blockerboards[s].push_back(bishop_blockermasks[s]);
                for (int i = 0; i < count; i++) {
                    if (!bitAt(bits, i)) bishop_blockerboards[s][bits] &= ~get_BB(poplsb(save));
                    else poplsb(save);
                }
            }
        }
    }

    //generate magics

    srand(342719832);
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int s = rc(r, c);

            bool found = false;
            while (!found) {
                bishop_magics[s] = rand_BB();
                found = true;

                for (BB blockerboard : bishop_blockerboards[s]) {
                    BB answer = getAnswer(r, c, blockerboard, NORTHEAST);
                    BB index = (blockerboard*bishop_magics[s])>>BISHOP_SHIFT;

                    if (bishop_table[s][index] == 0) bishop_table[s][index] = answer;
                    else if (bishop_table[s][index] != answer) {
                        for (int i = 0; i < (1<<BISHOP_BITS); i++) 
                            bishop_table[s][i] = 0;
                        found = false;     
                        break;
                    }
                }
            }

            found = false;
            while (!found) {
                rook_magics[s] = rand_BB();
                found = true;

                for (BB blockerboard : rook_blockerboards[s]) {
                    BB answer = getAnswer(r, c, blockerboard, NORTH);
                    BB index = (blockerboard*rook_magics[s])>>ROOK_SHIFT;

                    if (rook_table[s][index] == 0) rook_table[s][index] = answer;
                    else if (rook_table[s][index] != answer) {
                        for (int i = 0; i < (1<<ROOK_BITS); i++) 
                            rook_table[s][i] = 0;
                        found = false;
                        break;
                    }
                }
            }
        }
    }
}

vector<Move> get_legal_moves(Pos& pos) {
    assert(pos.ref_piece_mask(WHITE, KING) != 0);
    assert(pos.ref_piece_mask(BLACK, KING) != 0);

	pos.update_pins_and_checks();
    if (!pos.pi_log.back().has_updated_atks) pos.update_atks();

    vector<Move> moves;
    moves.reserve(Pos::MOVES_RESERVE_SIZE);

    if (pos.ref_num_checks() != 2) {
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
        if (pos.ref_moveable_squares(from) & get_BB(to))
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
    if (!pos.ref_piece_mask(pos.turn, PAWN)) return;

    BB pawns = pos.ref_piece_mask(pos.turn, PAWN);

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

        p1 = (pawns << 8) & ~pos.ref_occ();
        p2 = ((p1 & get_rank_mask(2)) << 8) & ~pos.ref_occ();
        rc = ((pawns & ~get_file_mask(7)) << 9);
        lc = ((pawns & ~get_file_mask(0)) << 7);
        if (pos.ref_ep() != SQUARE_NONE) {
            rc_ep = rc & get_BB(pos.ref_ep());
            lc_ep = lc & get_BB(pos.ref_ep());
        }
        rc &= pos.ref_occ(pos.notturn);
        lc &= pos.ref_occ(pos.notturn);

        p_p1 = p1 & get_rank_mask(7);
        p_rc = rc & get_rank_mask(7);
        p_lc = lc & get_rank_mask(7);

        p1 &= ~get_rank_mask(7);
        rc &= ~get_rank_mask(7);
        lc &= ~get_rank_mask(7);
    }
    else {
        p1_t = -8;
        rc_t = -9;
        lc_t = -7;

        p1 = (pawns >> 8) & ~pos.ref_occ();
        p2 = ((p1 & get_rank_mask(5)) >> 8) & ~pos.ref_occ();
        rc = ((pawns & ~get_file_mask(0)) >> 9);
        lc = ((pawns & ~get_file_mask(7)) >> 7);
        if (pos.ref_ep() != SQUARE_NONE) {
            rc_ep = rc & get_BB(pos.ref_ep());
            lc_ep = lc & get_BB(pos.ref_ep());
        }
        rc &= pos.ref_occ(pos.notturn);
        lc &= pos.ref_occ(pos.notturn);
        
        p_p1 = p1 & get_rank_mask(0);
        p_rc = rc & get_rank_mask(0);
        p_lc = lc & get_rank_mask(0);

        p1 &= ~get_rank_mask(0);
        rc &= ~get_rank_mask(0);
        lc &= ~get_rank_mask(0);
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
    BB knights = pos.ref_piece_mask(pos.turn, KNIGHT);

    while (knights) {
        int from = poplsb(knights);
        BB atk = get_knight_atk(from) & ~pos.ref_occ(pos.turn);
        BB cap = atk & pos.ref_occ(pos.notturn);
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
    BB bishop = pos.ref_piece_mask(pos.turn, BISHOP);

    while (bishop) {
        int from = poplsb(bishop);
        BB atk = get_bishop_atk(from, pos.ref_occ()) & ~pos.ref_occ(pos.turn);
        BB cap = atk & pos.ref_occ(pos.notturn);
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
    BB rooks = pos.ref_piece_mask(pos.turn, ROOK);

    while (rooks) {
        int from = poplsb(rooks);
        BB atk = get_rook_atk(from, pos.ref_occ()) & ~pos.ref_occ(pos.turn);
        BB cap = atk & pos.ref_occ(pos.notturn);
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
    BB queens = pos.ref_piece_mask(pos.turn, QUEEN);

    while (queens) {
        int from = poplsb(queens);
        BB atk = get_queen_atk(from, pos.ref_occ()) & ~pos.ref_occ(pos.turn);
        BB cap = atk & pos.ref_occ(pos.notturn);
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

const BB WKS_CLEARANCE = get_BB(F1) | get_BB(G1);
const BB WQS_CLEARANCE = get_BB(B1) | get_BB(C1) | get_BB(D1);

const BB BKS_CLEARANCE = get_BB(F8) | get_BB(G8);
const BB BQS_CLEARANCE = get_BB(B8) | get_BB(C8) | get_BB(D8);

const BB WKS_SAFE = get_BB(F1) | get_BB(G1);
const BB WQS_SAFE = get_BB(C1) | get_BB(D1);

const BB BKS_SAFE = get_BB(F8) | get_BB(G8);
const BB BQS_SAFE = get_BB(C8) | get_BB(D8);

void add_king_moves(vector<Move>& moves, Pos& pos) {
    int ksq = lsb(pos.ref_piece_mask(pos.turn, KING));
    BB atk = get_king_atk(ksq) & ~pos.ref_occ(pos.turn);

    if (atk == 0) return;

    BB notturn_atk = pos.ref_atk(pos.notturn);
    atk &= ~notturn_atk;

    BB cap = atk & pos.ref_occ(pos.notturn);
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
    if (pos.ref_num_checks() == 0 && pos.ref_cr()) {
        if (pos.turn == WHITE && (pos.ref_cr() &  0b0011)) {
            if (getWK(pos.ref_cr()) && !(pos.ref_occ() & WKS_CLEARANCE) && !(notturn_atk & WKS_SAFE)) moves.push_back(make_move(ksq, G1, KING_CASTLE));
            if (getWQ(pos.ref_cr()) && !(pos.ref_occ() & WQS_CLEARANCE) && !(notturn_atk & WQS_SAFE)) moves.push_back(make_move(ksq, C1, QUEEN_CASTLE));
        }
        else if (pos.turn == BLACK && (pos.ref_cr() &  0b1100)){
            if (getBK(pos.ref_cr()) && !(pos.ref_occ() & BKS_CLEARANCE) && !(notturn_atk & BKS_SAFE)) moves.push_back(make_move(ksq, G8, KING_CASTLE));
            if (getBQ(pos.ref_cr()) && !(pos.ref_occ() & BQS_CLEARANCE) && !(notturn_atk & BQS_SAFE)) moves.push_back(make_move(ksq, C8, QUEEN_CASTLE));
        }
    }
}
