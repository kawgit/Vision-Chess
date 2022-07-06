#pragma once

#include "pos.h"
#include "types.h"
#include "bits.h"
#include <vector>

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

inline BB& getPawnAtk(Color c, Square s) { return pawn_atks[c - BLACK][s]; }
inline BB getKnightAtk(Square s) { return knight_atks[s]; }
inline BB getRookAtk(Square s, BB occupied) { return rook_table[s][(rook_magics[s]*(occupied & rook_blockermasks[s]))>>ROOK_SHIFT]; }
inline BB getBishopAtk(Square s, BB occupied) { return bishop_table[s][(bishop_magics[s]*(occupied & bishop_blockermasks[s]))>>BISHOP_SHIFT]; }
inline BB getQueenAtk(Square s, BB occupied) { return getRookAtk(s, occupied) | getBishopAtk(s, occupied); }
inline BB getKingAtk(Square s) { return king_atks[s]; }

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

vector<Move> getLegalMoves(Pos& p);

void addPawnMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addKnightMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addBishopMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addRookMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addQueenMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addKingMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);

