#pragma once

#include "pos.h"
#include "types.h"
#include "bits.h"
#include <vector>
#include <cassert>

using namespace std;

const int ROOK_BITS = 20;
const int BISHOP_BITS = 20;

const int ROOK_SHIFT = 64-ROOK_BITS;
const int BISHOP_SHIFT = 64-BISHOP_BITS;

extern BB bishop_magics[64];
extern BB rook_magics[64];
extern BB bishop_table[64][1<<BISHOP_BITS];
extern BB rook_table[64][1<<ROOK_BITS];

extern BB rays[64][8];

extern BB rook_atks[64];
extern BB bishop_atks[64];

extern BB rook_blockermasks[64];
extern BB bishop_blockermasks[64];

extern BB pawn_atks[2][64];
extern BB knight_atks[64];
extern BB king_atks[64];

inline BB& get_pawn_atk(Color c, Square s) { assert(s < 64); return pawn_atks[c - BLACK][s]; }

inline BB get_knight_atk(Square s) { assert(s < 64); return knight_atks[s]; }

inline BB get_rook_atk(Square s, BB occupied) { assert(s < 64); return rook_table[s][(rook_magics[s]*(occupied & rook_blockermasks[s]))>>ROOK_SHIFT]; }

inline BB get_bishop_atk(Square s, BB occupied) { assert(s < 64); return bishop_table[s][(bishop_magics[s]*(occupied & bishop_blockermasks[s]))>>BISHOP_SHIFT]; }

inline BB get_queen_atk(Square s, BB occupied) { assert(s < 64); return get_rook_atk(s, occupied) | get_bishop_atk(s, occupied); }

inline BB get_king_atk(Square s) { assert(s < 64); return king_atks[s]; }


void initMoveGen(int seed = 38921083);

struct PosInfo { //pins and checks
    BB occ;
    BB notturn_occ;
    BB turn_occ;

    int checks = 0;
    BB check_blocking_squares = ~0ULL;
    BB moveable_squares[64] = {};
    BB pinned_mask = 0ULL;
    bool isPawnCheck = false;

    PosInfo(Pos &p);
    void add_check(BB block_squares);
    void add_pin(int square, BB moveable_squares_);
    bool is_moveable(int from, int to);
};

void addPawnMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);

void addKnightMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);

void addBishopMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);

void addRookMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);

void addQueenMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);

void addKingMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);


