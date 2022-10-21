#include "pos.h"
#include "types.h"
#include "bits.h"
#include "movegen.h"
#include <vector>

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

void initMoveGen(int seed) {

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

PosInfo::PosInfo(Pos &p) {
    Color turn = p.turn;

    turn_occ = p.get_occ(turn);
    notturn_occ = p.get_occ(p.notturn);
    occ = turn_occ | notturn_occ;

    int ksq = lsb(p.pieces(turn, KING));


    if (get_pawn_atk(turn, ksq) & p.pieces(p.notturn, PAWN)) {
        checks++;
        check_blocking_squares &= get_pawn_atk(turn, ksq) & p.pieces(p.notturn, PAWN);
        isPawnCheck = true;
    }
    else if (get_knight_atk(ksq) & p.pieces(p.notturn, KNIGHT)) {
        checks++;
        check_blocking_squares &= get_knight_atk(ksq) & p.pieces(p.notturn, KNIGHT);
    }

    BB bishop_sliders = p.pieces(p.notturn, BISHOP) | p.pieces(p.notturn, QUEEN);
    BB total_bishop_rays = get_bishop_atk(ksq, occ);
    BB bishop_checkers = bishop_sliders & total_bishop_rays;
    BB notturn_bishop_rays = get_bishop_atk(ksq, notturn_occ);
    BB bishop_pinners = bishop_sliders & notturn_bishop_rays & ~bishop_checkers;
    if (bishop_checkers) {
        for (int d = NORTHEAST; d <= NORTHWEST; d+=2) {
            BB &ray = rays[ksq][d];
            if (ray & bishop_checkers) {
                add_check(ray & total_bishop_rays);
                //break; //ASSUMPTION: THERE CAN BE NO DOUBLE CHECK WITH 2 BISHOP SLIDERS
            }
        }
    }

    if (bishop_pinners) {
        for (int d = NORTHEAST; d <= NORTHWEST && bishop_pinners; d+=2) {
            BB &ray = rays[ksq][d];
            BB ally_mask = ray & notturn_bishop_rays & turn_occ;
            if (ray & bishop_pinners) {
                bishop_pinners &= ~ray;
                if (bitcount(ally_mask) == 1)
                    add_pin(lsb(ally_mask), notturn_bishop_rays & ray);
            }
        }
    }

    BB rook_sliders = p.pieces(p.notturn, ROOK) | p.pieces(p.notturn, QUEEN);
    BB total_rook_rays = get_rook_atk(ksq, occ);
    BB rook_checkers = rook_sliders & total_rook_rays;
    BB notturn_rook_rays = get_rook_atk(ksq, notturn_occ);
    BB rook_pinners = rook_sliders & notturn_rook_rays & ~rook_checkers;
    if (rook_checkers) {
        for (int d = NORTH; d <= WEST; d+=2) {
            BB &ray = rays[ksq][d];
            if (ray & rook_checkers) {
                add_check(ray & total_rook_rays);
                //break; //ASSUMPTION: THERE CAN BE NO DOUBLE CHECK WITH 2 ROOK SLIDERS
            }
        }
    }

    if (rook_pinners) {
        for (int d = NORTH; d <= WEST && rook_pinners; d+=2) {
            BB &ray = rays[ksq][d];
            BB ally_mask = ray & notturn_rook_rays & turn_occ;
            if (ray & rook_pinners) {
                rook_pinners &= ~ray;
                if (bitcount(ally_mask) == 1)
                    add_pin(lsb(ally_mask), notturn_rook_rays & ray);
            }
        }
    }

    //EN PASSANT CASE
    if (p.ep != SQUARE_NONE && checks != 2) {
        BB to_pawns = get_pawn_atk(p.notturn, p.ep) & p.pieces(turn, PAWN);
        while (to_pawns) {
            int from = poplsb(to_pawns);
            BB post = (occ | get_BB(p.ep)) & (~get_BB(from)) & (~get_BB(p.ep - (turn == WHITE ? 8 : -8)));
            if ((checks == 1 && !isPawnCheck) || (get_bishop_atk(ksq, post) & bishop_sliders) || (get_rook_atk(ksq, post) & rook_sliders)) {
                if (pinned_mask & get_BB(from))
                    moveable_squares[from] &= ~get_BB(p.ep);
                else {
                    moveable_squares[from] = ~get_BB(p.ep);
                    pinned_mask |= get_BB(from);
                }
            }
            else {
                moveable_squares[from] |= get_BB(p.ep);
            }
        }
    }
}

inline void PosInfo::add_check(BB block_squares) {
    check_blocking_squares &= block_squares;
    checks++;
}
inline void PosInfo::add_pin(int square, BB moveable_squares_) {
    moveable_squares[square] = moveable_squares_;
    pinned_mask |= get_BB(square);
}
inline bool PosInfo::is_moveable(int from, int to) {
    return ((!(pinned_mask & get_BB(from))) || (moveable_squares[from] & get_BB(to))) && (check_blocking_squares & get_BB(to)); 
}

vector<Move> get_legal_moves(Pos& p) {
    PosInfo posInfo(p);
    
    vector<Move> moves;
    moves.reserve(Pos::RESERVE_SIZE);

    if (posInfo.checks != 2) {
        addPawnMoves(moves, p, posInfo);
        addKnightMoves(moves, p, posInfo);
        addBishopMoves(moves, p, posInfo);
        addRookMoves(moves, p, posInfo);
        addQueenMoves(moves, p, posInfo);
    }
    addKingMoves(moves, p, posInfo);
    
    return moves;
}

inline void addPawnMovesFromMask(vector<Move> &moves, PosInfo &posInfo, BB &mask, int transform, MoveFlag flags) {
    while (mask) {
        int to = poplsb(mask);
        int from = to - transform;
        if (posInfo.is_moveable(from, to))
            moves.push_back(make_move(from, to, flags));
    }
}
inline void addPawnEpMovesFromMask(vector<Move> &moves, PosInfo &posInfo, BB &mask, int transform) {
    while (mask) {
        int to = poplsb(mask);
        int from = to - transform;
        if (posInfo.moveable_squares[from] & get_BB(to))
            moves.push_back(make_move(from, to, EP));
    }
}
inline void addPawnPromotionMovesFromMask(vector<Move> &moves, PosInfo &posInfo, BB &mask, int transform, MoveFlag flags) {
    while (mask) {
        int to = poplsb(mask);
        int from = to - transform;
        if (posInfo.is_moveable(from, to))
            for (MoveFlag prom_flag = N_PROM; prom_flag <= Q_PROM; prom_flag++)
                moves.push_back(make_move(from, to, flags | prom_flag));
    }
}

void addPawnMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo) {
    if (!p.pieces(p.turn, PAWN)) return;

    BB pawns = p.pieces(p.turn, PAWN);

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

    if (p.turn == WHITE) {
        p1_t = 8;
        rc_t = 9;
        lc_t = 7;

        p1 = (pawns << 8) & ~posInfo.occ;
        p2 = ((p1 & get_rank_mask(2)) << 8) & ~posInfo.occ;
        rc = ((pawns & ~get_file_mask(7)) << 9);
        lc = ((pawns & ~get_file_mask(0)) << 7);
        if (p.ep != SQUARE_NONE) {
            rc_ep = rc & get_BB(p.ep);
            lc_ep = lc & get_BB(p.ep);
        }
        rc &= posInfo.notturn_occ;
        lc &= posInfo.notturn_occ;

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

        p1 = (pawns >> 8) & ~posInfo.occ;
        p2 = ((p1 & get_rank_mask(5)) >> 8) & ~posInfo.occ;
        rc = ((pawns & ~get_file_mask(0)) >> 9);
        lc = ((pawns & ~get_file_mask(7)) >> 7);
        if (p.ep != SQUARE_NONE) {
            rc_ep = rc & get_BB(p.ep);
            lc_ep = lc & get_BB(p.ep);
        }
        rc &= posInfo.notturn_occ;
        lc &= posInfo.notturn_occ;
        
        p_p1 = p1 & get_rank_mask(0);
        p_rc = rc & get_rank_mask(0);
        p_lc = lc & get_rank_mask(0);

        p1 &= ~get_rank_mask(0);
        rc &= ~get_rank_mask(0);
        lc &= ~get_rank_mask(0);
    }

    addPawnMovesFromMask(moves, posInfo, p1, p1_t, QUIET);
    addPawnMovesFromMask(moves, posInfo, p2, p1_t + p1_t, DOUBLE_PAWN_PUSH);
    addPawnMovesFromMask(moves, posInfo, rc, rc_t, CAPTURE);
    addPawnMovesFromMask(moves, posInfo, lc, lc_t, CAPTURE);
    addPawnEpMovesFromMask(moves, posInfo, rc_ep, rc_t);
    addPawnEpMovesFromMask(moves, posInfo, lc_ep, lc_t);

    addPawnPromotionMovesFromMask(moves, posInfo, p_p1, p1_t, QUIET);
    addPawnPromotionMovesFromMask(moves, posInfo, p_rc, rc_t, CAPTURE);
    addPawnPromotionMovesFromMask(moves, posInfo, p_lc, lc_t, CAPTURE);
}
void addKnightMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo) {
    BB knights = p.pieces(p.turn, KNIGHT);

    while (knights) {
        int from = poplsb(knights);
        BB atk = get_knight_atk(from) & ~posInfo.turn_occ;
        BB cap = atk & posInfo.notturn_occ;
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (posInfo.is_moveable(from, to)) moves.push_back(make_move(from, to, CAPTURE));
        }
        while (qui) {
            int to = poplsb(qui);
            if (posInfo.is_moveable(from, to)) moves.push_back(make_move(from, to, QUIET));
        }
    }
}
void addBishopMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo) {
    BB bishop = p.pieces(p.turn, BISHOP);

    while (bishop) {
        int from = poplsb(bishop);
        BB atk = get_bishop_atk(from, posInfo.occ) & ~posInfo.turn_occ;
        BB cap = atk & posInfo.notturn_occ;
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (posInfo.is_moveable(from, to)) moves.push_back(make_move(from, to, CAPTURE));
        }
        while (qui) {
            int to = poplsb(qui);
            if (posInfo.is_moveable(from, to)) moves.push_back(make_move(from, to, QUIET));
        }
    }
}
void addRookMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo) {
    BB rooks = p.pieces(p.turn, ROOK);

    while (rooks) {
        int from = poplsb(rooks);
        BB atk = get_rook_atk(from, posInfo.occ) & ~posInfo.turn_occ;
        BB cap = atk & posInfo.notturn_occ;
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (posInfo.is_moveable(from, to)) moves.push_back(make_move(from, to, CAPTURE));
        }
        while (qui) {
            int to = poplsb(qui);
            if (posInfo.is_moveable(from, to)) moves.push_back(make_move(from, to, QUIET));
        }
    }
}
void addQueenMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo) {
    BB queens = p.pieces(p.turn, QUEEN);

    while (queens) {
        int from = poplsb(queens);
        BB atk = get_queen_atk(from, posInfo.occ) & ~posInfo.turn_occ;
        BB cap = atk & posInfo.notturn_occ;
        BB qui = atk & ~cap;
        while (cap) {
            int to = poplsb(cap);
            if (posInfo.is_moveable(from, to)) moves.push_back(make_move(from, to, CAPTURE));
        }
        while (qui) {
            int to = poplsb(qui);
            if (posInfo.is_moveable(from, to)) moves.push_back(make_move(from, to, QUIET));
        }
    }
}
void addKingMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo) {
    int ksq = lsb(p.pieces(p.turn, KING));
    BB atk = get_king_atk(ksq) & ~posInfo.turn_occ;

    if (atk == 0) return;

    BB notturn_atk = p.get_atk_mask(opp(p.turn));

    atk &= ~notturn_atk;

    BB cap = atk & posInfo.notturn_occ;
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

    const static BB WKS_CLEARANCE = get_BB(F1) | get_BB(G1);
    const static BB WQS_CLEARANCE = get_BB(B1) | get_BB(C1) | get_BB(D1);

    const static BB BKS_CLEARANCE = get_BB(F8) | get_BB(G8);
    const static BB BQS_CLEARANCE = get_BB(B8) | get_BB(C8) | get_BB(D8);

    const static BB WKS_SAFE = get_BB(F1) | get_BB(G1);
    const static BB WQS_SAFE = get_BB(C1) | get_BB(D1);

    const static BB BKS_SAFE = get_BB(F8) | get_BB(G8);
    const static BB BQS_SAFE = get_BB(C8) | get_BB(D8);

    if (posInfo.checks == 0 && p.cr) {
        if (p.turn == WHITE && (p.cr &  0b0011)) {
            if (getWK(p.cr) && !(posInfo.occ & WKS_CLEARANCE) && !(notturn_atk & WKS_SAFE)) moves.push_back(make_move(ksq, G1, KING_CASTLE));
            if (getWQ(p.cr) && !(posInfo.occ & WQS_CLEARANCE) && !(notturn_atk & WQS_SAFE)) moves.push_back(make_move(ksq, C1, QUEEN_CASTLE));
        }
        else if (p.turn == BLACK && (p.cr &  0b1100)){
            if (getBK(p.cr) && !(posInfo.occ & BKS_CLEARANCE) && !(notturn_atk & BKS_SAFE)) moves.push_back(make_move(ksq, G8, KING_CASTLE));
            if (getBQ(p.cr) && !(posInfo.occ & BQS_CLEARANCE) && !(notturn_atk & BQS_SAFE)) moves.push_back(make_move(ksq, C8, QUEEN_CASTLE));
        }
    }
}
