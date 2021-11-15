#include "movegen.h"
#include "pos.h"
#include "util.h"
#include "types.h"
#include <iostream>
#include <vector>

using namespace std;

BB bishop_magics[64];
BB rook_magics[64];
BB bishop_table[64][1<<BISHOP_BITS];
BB rook_table[64][1<<ROOK_BITS];

BB rays[64][8];

BB rook_moves[64];
BB bishop_moves[64];

BB rook_blockermasks[64];
BB bishop_blockermasks[64];

BB pawn_moves[2][64];
BB knight_moves[64];
BB king_moves[64];

int directions[8][2] = {{0, 1},{1, 1},{1, 0},{1, -1},{0, -1},{-1, -1},{-1, 0},{-1, 1}};

BB getAnswer(int r, int c, BB blockerboard, int d_offset) {
    BB answer = 0;
    for (int d = d_offset; d < 8; d+=2) {
        for (int m = 1; m <= 7; m++) {
            int _r = r + directions[d][1]*m;
            int _c = c + directions[d][0]*m;
            int _s = RC2SQ(_r, _c);
            if (_r >= 0 && _r <= 7 && _c >= 0 && _c <= 7) {
                answer |= getBB(_s);
                if (bitAt(blockerboard, _s)) break;
            }
            else break;
        }
    }
    return answer;
}

BB WKS_CLEARANCE;
BB WKS_SAFE;
BB WQS_CLEARANCE;
BB WQS_SAFE;

BB BKS_CLEARANCE;
BB BKS_SAFE;
BB BQS_CLEARANCE;
BB BQS_SAFE;

void initM(int seed) {

    //generate rays and moves

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int s = RC2SQ(r, c);
            for (int d = 0; d < 8; d++) {
                for (int m = 1; m <= 7; m++) {
                    int _r = r + directions[d][1]*m;
                    int _c = c + directions[d][0]*m;
                    if (_r >= 0 && _r <= 7 && _c >= 0 && _c <= 7) {
                        rays[s][d] |= getBB(RC2SQ(_r, _c));
                        if (m == 1) king_moves[s] |= getBB(RC2SQ(_r, _c));
                    }
                }
            }
        }
    }

    for (int s = 0; s < 64; s++) {
        rook_moves[s] = rays[s][NORTH] | rays[s][EAST] | rays[s][SOUTH] | rays[s][WEST];
        bishop_moves[s] = rays[s][NORTHEAST] | rays[s][SOUTHEAST] | rays[s][SOUTHWEST] | rays[s][NORTHWEST];
        pawn_moves[WHITE][s] = (rays[s][NORTHEAST] | rays[s][NORTHWEST]) & king_moves[s];
        pawn_moves[BLACK][s] = (rays[s][SOUTHEAST] | rays[s][SOUTHWEST]) & king_moves[s];
    }

    int knight_directions[8][2] = {{1, 2},{2, 1},{2, -1},{1, -2},{-1, -2},{-2, -1},{-2, 1},{-1, 2}};
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int s = RC2SQ(r, c);
            for (int d = 0; d < 8; d++) {
                int _r = r + knight_directions[d][1];
                int _c = c + knight_directions[d][0];

                if (_r >= 0 && _r <= 7 && _c >= 0 && _c <= 7) 
                    knight_moves[s] |= getBB(RC2SQ(_r, _c));
            }
        }
    }

    //generate blockermasks

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            BB exclude = 0ULL;
            if (c != 0) exclude |= file_masks[0];
            if (c != 7) exclude |= file_masks[7];
            if (r != 0) exclude |= rank_masks[0];
            if (r != 7) exclude |= rank_masks[7];

            int s = RC2SQ(r, c);
            rook_blockermasks[s] = rook_moves[s] & ~exclude;
            bishop_blockermasks[s] = bishop_moves[s] & ~exclude;
        }
    }

    //generate blockerboards

    vector<BB> rook_blockerboards[64];
    vector<BB> bishop_blockerboards[64];
    for (int s = 0; s < 64; s++) {
        {
            int count = bitcount(rook_blockermasks[s]);
            for (uint32_t bits = 0; bits < (1ULL<<count); bits++) {
                BB save = rook_blockermasks[s];
                rook_blockerboards[s].push_back(rook_blockermasks[s]);
                for (int i = 0; i < count; i++)
                    if (!bitAt(bits, i)) rook_blockerboards[s][bits] &= ~getBB(poplsb(save));
                    else poplsb(save);
            }
        }

        {        
            int count = bitcount(bishop_blockermasks[s]);
            for (uint32_t bits = 0; bits < (1ULL<<count); bits++) {
                BB save = bishop_blockermasks[s];
                bishop_blockerboards[s].push_back(bishop_blockermasks[s]);
                for (int i = 0; i < count; i++)
                    if (!bitAt(bits, i)) bishop_blockerboards[s][bits] &= ~getBB(poplsb(save));
                    else poplsb(save);
            }
        }
    }

    //generate magics

    srand(seed);
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int s = RC2SQ(r, c);

            bool found = false;
            while (!found) {
                bishop_magics[s] = randBB();
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
                rook_magics[s] = randBB();
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

    WKS_CLEARANCE = getBB(F1) | getBB(G1);
    WQS_CLEARANCE = getBB(B1) | getBB(C1) | getBB(D1);

    BKS_CLEARANCE = getBB(F8) | getBB(G8);
    BQS_CLEARANCE = getBB(B8) | getBB(C8) | getBB(D8);

    WKS_SAFE = getBB(F1) | getBB(G1);
    WQS_SAFE = getBB(C1) | getBB(D1);

    BKS_SAFE = getBB(F8) | getBB(G8);
    BQS_SAFE = getBB(C8) | getBB(D8);
}

BB getPawnAtk(Color c, Square s) {
    return pawn_moves[c][s];
}
BB getKnightAtk(Square s) {
    return knight_moves[s];
}
BB getRookAtk(Square s, BB &occupied) {
    return rook_table[s][(rook_magics[s]*(occupied & rook_blockermasks[s]))>>ROOK_SHIFT];
}
BB getBishopAtk(Square s, BB &occupied) {
    return bishop_table[s][(bishop_magics[s]*(occupied & bishop_blockermasks[s]))>>BISHOP_SHIFT];
}
BB getQueenAtk(Square s, BB &occupied) {
    return getRookAtk(s, occupied) | getBishopAtk(s, occupied);
}
BB getKingAtk(Square s) {
    return king_moves[s];
}

void addLegalMoves(Pos &p, vector<Move> &moves) {
    p.updateMasks();
    PNC pnc = getPNC(p);

    moves.reserve(100);

    if (pnc.checks != 2) {
        addPawnMoves(moves, p, pnc);
        addKnightMoves(moves, p, pnc);
        addBishopMoves(moves, p, pnc);
        addRookMoves(moves, p, pnc);
        addQueenMoves(moves, p, pnc);
    }
    addKingMoves(moves, p, pnc);

    p.inCheck = (pnc.checks != 0);
}

PNC getPNC(Pos &p) {
    PNC pnc;


    int ksq = lsb(p.getPieceMask(p.turn, KING));

    if (getPawnAtk(p.turn, ksq) & p.getPieceMask(p.notturn, PAWN)) {
        pnc.checks++;
        pnc.check_block_squares &= getPawnAtk(p.turn, ksq) & p.getPieceMask(p.notturn, PAWN);
        pnc.isPawnCheck = true;
    }
    else if (getKnightAtk(ksq) & p.getPieceMask(p.notturn, KNIGHT)) {
        pnc.checks++;
        pnc.check_block_squares &= getKnightAtk(ksq) & p.getPieceMask(p.notturn, KNIGHT);
    }

    BB bishop_sliders = p.getPieceMask(p.notturn, BISHOP) | p.getPieceMask(p.notturn, QUEEN);
    
    BB total_bishop_rays = getBishopAtk(ksq, p.occ);
    BB bishop_checkers = bishop_sliders & total_bishop_rays;

    BB notturn_bishop_rays = getBishopAtk(ksq, p.notturn_occ);
    BB bishop_pinners = bishop_sliders & notturn_bishop_rays & ~bishop_checkers;

    if (bishop_checkers) {
        for (int d = NORTHEAST; d <= NORTHWEST; d+=2) {
            BB &ray = rays[ksq][d];
            if (ray & bishop_checkers) {
                pnc.add_check(ray & total_bishop_rays);
                break; //ASSUMPTION: THERE CAN BE NO DOUBLE CHECK WITH 2 BISHOP SLIDERS
            }
        }
    }

    if (bishop_pinners) {
        for (int d = NORTHEAST; d <= NORTHWEST && bishop_pinners; d+=2) {
            BB &ray = rays[ksq][d];
            BB ally_mask = ray & notturn_bishop_rays & p.turn_occ;
            if (ray & bishop_pinners) {
                bishop_pinners &= ~ray;
                if (bitcount(ally_mask) == 1)
                    pnc.add_pin(lsb(ally_mask), notturn_bishop_rays & ray);
            }
        }
    }

    BB rook_sliders = p.getPieceMask(p.notturn, ROOK) | p.getPieceMask(p.notturn, QUEEN);
    
    BB total_rook_rays = getRookAtk(ksq, p.occ);
    BB rook_checkers = rook_sliders & total_rook_rays;

    BB notturn_rook_rays = getRookAtk(ksq, p.notturn_occ);
    BB rook_pinners = rook_sliders & notturn_rook_rays & ~rook_checkers;

    if (rook_checkers) {
        for (int d = NORTH; d <= WEST; d+=2) {
            BB &ray = rays[ksq][d];
            if (ray & rook_checkers) {
                pnc.add_check(ray & total_rook_rays);
                break; //ASSUMPTION: THERE CAN BE NO DOUBLE CHECK WITH 2 ROOK SLIDERS
            }
        }
    }

    if (rook_pinners) {
        for (int d = NORTH; d <= WEST && rook_pinners; d+=2) {
            BB &ray = rays[ksq][d];
            BB ally_mask = ray & notturn_rook_rays & p.turn_occ;
            if (ray & rook_pinners) {
                rook_pinners &= ~ray;
                if (bitcount(ally_mask) == 1)
                    pnc.add_pin(lsb(ally_mask), notturn_rook_rays & ray);
            }
        }
    }

    //EN PASSANT CASE
    if (p.ep < 64 && pnc.checks != 2) {
        BB to_pawns = getPawnAtk(p.notturn, p.ep) & p.getPieceMask(p.turn, PAWN);
        while (to_pawns) {
            int from = poplsb(to_pawns);
            BB post = (p.occ | getBB(p.ep)) & (~getBB(from)) & (~getBB(p.ep - (p.turn == WHITE ? 8 : -8)));
            if ((pnc.checks == 1 && !pnc.isPawnCheck) || (getBishopAtk(ksq, post) & bishop_sliders) || (getRookAtk(ksq, post) & rook_sliders)) {
                if (pnc.pinned_mask & getBB(from))
                    pnc.moveable_squares[from] &= ~getBB(p.ep);
                else {
                    pnc.moveable_squares[from] = ~getBB(p.ep);
                    pnc.pinned_mask |= getBB(from);
                }
            }
        }
    }

    return pnc;
}

void addPawnMoves(vector<Move> &moves, Pos &p, PNC &pnc) {

    if (!p.getPieceMask(p.turn, PAWN)) return;

    BB pawns = p.getPieceMask(p.turn, PAWN);

    BB p1;
    BB p2;
    BB rc;
    BB lc;
    BB rc_ep = 0ULL;
    BB lc_ep = 0ULL;
    
    BB p_p1;
    BB p_rc;
    BB p_lc;


    if (p.turn == WHITE) {
        p1 = (pawns << 8) & ~p.occ;
        p2 = ((p1 & rank_masks[2]) << 8) & ~p.occ;
        rc = ((pawns & ~file_masks[7]) << 9);
        lc = ((pawns & ~file_masks[0]) << 7);
        if (p.ep < 64) {
            rc_ep = rc & getBB(p.ep);
            lc_ep = lc & getBB(p.ep);
        }
        rc &= p.notturn_occ;
        lc &= p.notturn_occ;

        p_p1 = p1 & rank_masks[7];
        p_rc = rc & rank_masks[7];
        p_lc = lc & rank_masks[7];

        p1 &= ~rank_masks[7];
        rc &= ~rank_masks[7];
        lc &= ~rank_masks[7];
    }
    else {
        p1 = (pawns >> 8) & ~p.occ;
        p2 = ((p1 & rank_masks[5]) >> 8) & ~p.occ;
        rc = ((pawns & ~file_masks[0]) >> 9);
        lc = ((pawns & ~file_masks[7]) >> 7);
        if (p.ep < 64) {
            rc_ep = rc & getBB(p.ep);
            lc_ep = lc & getBB(p.ep);
        }
        rc &= p.notturn_occ;
        lc &= p.notturn_occ;
        
        p_p1 = p1 & rank_masks[0];
        p_rc = rc & rank_masks[0];
        p_lc = lc & rank_masks[0];

        p1 &= ~rank_masks[0];
        rc &= ~rank_masks[0];
        lc &= ~rank_masks[0];
    }

    int t = (p.turn == WHITE ? 1 : -1);
    int p1_t = 8 * t;
    int rc_t = 9 * t;
    int lc_t = 7 * t;

    while (p_rc) {
        Square to = poplsb(p_rc);
        Square from = to - rc_t;
        if (pnc.is_moveable(from, to)) {
            for (int i = QUEEN - 1; i >= PAWN; i--) {
                moves.emplace_back(from, to, PAWN, p.getPieceAt(to, p.notturn), (N_PROM_F + i) | CAPTURE_F);
            }
        }
    }
    while (p_lc) {
        Square to = poplsb(p_lc);
        Square from = to - lc_t;
        if (pnc.is_moveable(from, to)) {
            for (int i = QUEEN - 1; i >= PAWN; i--) {
                moves.emplace_back(from, to, PAWN, p.getPieceAt(to, p.notturn), (N_PROM_F + i) | CAPTURE_F);
            }
        }
    }
    while (p_p1) {
        Square to = poplsb(p_p1);
        Square from = to - p1_t;
        if (pnc.is_moveable(from, to)) {
            for (int i = QUEEN - 1; i >= PAWN; i--) {
                moves.emplace_back(from, to, PAWN, NO_P, (N_PROM_F + i));
            }
        }
    }

    while (rc) {
        Square to = poplsb(rc);
        Square from = to - rc_t;
        if (pnc.is_moveable(from, to))
            moves.emplace_back(from, to, PAWN, p.getPieceAt(to, p.notturn), CAPTURE_F);
    }
    while (lc) {
        Square to = poplsb(lc);
        Square from = to - lc_t;
        if (pnc.is_moveable(from, to))
            moves.emplace_back(from, to, PAWN, p.getPieceAt(to, p.notturn), CAPTURE_F);
    }

    while (rc_ep) {
        Square to = poplsb(rc_ep);
        Square from = to - rc_t;
        if (((!(pnc.pinned_mask & getBB(from))) || (pnc.moveable_squares[from] & getBB(to))) && (pnc.checks == 0 || pnc.isPawnCheck))
            moves.emplace_back(from, to, PAWN, NO_P, EP_F);
    }

    while (lc_ep) {
        Square to = poplsb(lc_ep);
        Square from = to - lc_t;
        if (((!(pnc.pinned_mask & getBB(from))) || (pnc.moveable_squares[from] & getBB(to))) && (pnc.checks == 0 || pnc.isPawnCheck))
            moves.emplace_back(from, to, PAWN, NO_P, EP_F);
    }

    while (p1) {
        Square to = poplsb(p1);
        Square from = to - p1_t;
        if (pnc.is_moveable(from, to))
            moves.emplace_back(from, to, PAWN, NO_P, QUIET_F);
    }
    while (p2) {
        Square to = poplsb(p2);
        Square from = to - p1_t - p1_t;
        if (pnc.is_moveable(from, to))
            moves.emplace_back(from, to, PAWN, NO_P, DOUBLE_PAWN_PUSH_F);
    }
}

void addKnightMoves(vector<Move> &moves, Pos &p, PNC &pnc) {
    BB knights = p.getPieceMask(p.turn, KNIGHT);

    while (knights) {
        int from = poplsb(knights);
        BB atk = getKnightAtk(from) & ~p.turn_occ;
        BB cap = atk & p.notturn_occ;
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (pnc.is_moveable(from, to)) moves.emplace_back(from, to, KNIGHT, p.getPieceAt(to, p.notturn), CAPTURE_F);
        }
        while (qui) {
            int to = poplsb(qui);
            if (pnc.is_moveable(from, to)) moves.emplace_back(from, to, KNIGHT, NO_P, QUIET_F);
        }
    }
}

void addBishopMoves(vector<Move> &moves, Pos &p, PNC &pnc) {
    BB bishop = p.getPieceMask(p.turn, BISHOP);

    while (bishop) {
        int from = poplsb(bishop);
        BB atk = getBishopAtk(from, p.occ) & ~p.turn_occ;
        BB cap = atk & p.notturn_occ;
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (pnc.is_moveable(from, to)) moves.emplace_back(from, to, BISHOP, p.getPieceAt(to, p.notturn), CAPTURE_F);
        }
        while (qui) {
            int to = poplsb(qui);
            if (pnc.is_moveable(from, to)) moves.emplace_back(from, to, BISHOP, NO_P, QUIET_F);
        }
    }
}

void addRookMoves(vector<Move> &moves, Pos &p, PNC &pnc) {
    BB rooks = p.getPieceMask(p.turn, ROOK);

    while (rooks) {
        int from = poplsb(rooks);
        BB atk = getRookAtk(from, p.occ) & ~p.turn_occ;
        BB cap = atk & p.notturn_occ;
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (pnc.is_moveable(from, to)) moves.emplace_back(from, to, ROOK, p.getPieceAt(to, p.notturn), CAPTURE_F);
        }
        while (qui) {
            int to = poplsb(qui);
            if (pnc.is_moveable(from, to)) moves.emplace_back(from, to, ROOK, NO_P, QUIET_F);
        }
    }
}

void addQueenMoves(vector<Move> &moves, Pos &p, PNC &pnc) {
    BB queens = p.getPieceMask(p.turn, QUEEN);

    while (queens) {
        int from = poplsb(queens);
        BB atk = getQueenAtk(from, p.occ) & ~p.turn_occ;
        BB cap = atk & p.notturn_occ;
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (pnc.is_moveable(from, to)) moves.emplace_back(from, to, QUEEN, p.getPieceAt(to, p.notturn), CAPTURE_F);
        }
        while (qui) {
            int to = poplsb(qui);
            if (pnc.is_moveable(from, to)) moves.emplace_back(from, to, QUEEN, NO_P, QUIET_F);
        }
    }
}

void addKingMoves(vector<Move> &moves, Pos &p, PNC &pnc) {
    int ksq = lsb(p.getPieceMask(p.turn, KING));
    BB atk = getKingAtk(ksq) & ~p.turn_occ;

    if (atk == 0) return;

    BB notturn_atk = p.getAtkMask(p.notturn);

    atk &= ~notturn_atk;

    BB cap = atk & p.notturn_occ;
    BB qui = atk & ~cap;

    while (cap) {
        int to = poplsb(cap);
        moves.emplace_back(ksq, to, KING, p.getPieceAt(to, p.notturn), CAPTURE_F);
    }
    while (qui) {
        int to = poplsb(qui);
        moves.emplace_back(ksq, to, KING, NO_P, QUIET_F);
    }

    //CASTLING

    if (pnc.checks == 0 && p.cr.bits) {
        if (p.turn == WHITE && (p.cr.bits &  0b0011)) {
            if (p.cr.getWK() && !(p.occ & WKS_CLEARANCE) && !(notturn_atk & WKS_SAFE)) moves.emplace_back(ksq, G1, KING, NO_P, KING_CASTLE_F);
            if (p.cr.getWQ() && !(p.occ & WQS_CLEARANCE) && !(notturn_atk & WQS_SAFE)) moves.emplace_back(ksq, C1, KING, NO_P, QUEEN_CASTLE_F);
        }
        else if (p.turn == BLACK && (p.cr.bits &  0b1100)){
            if (p.cr.getBK() && !(p.occ & BKS_CLEARANCE) && !(notturn_atk & BKS_SAFE)) moves.emplace_back(ksq, G8, KING, NO_P, KING_CASTLE_F);
            if (p.cr.getBQ() && !(p.occ & BQS_CLEARANCE) && !(notturn_atk & BQS_SAFE)) moves.emplace_back(ksq, C8, KING, NO_P, QUEEN_CASTLE_F);
        }
    }
}
