#pragma once

#include "pos.h"
#include "types.h"
#include "bits.h"
#include "move.h"
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

inline BB get_piece_atk(Piece pt, Square s, Color c, BB occupied) { 
    assert(s < 64);
    switch (pt) {
        case PAWN:
            return get_pawn_atk(c, s);
        case KNIGHT:
            return get_knight_atk(s);

        case BISHOP:
            return get_bishop_atk(s, occupied);

        case ROOK:
            return get_rook_atk(s, occupied);

        case QUEEN:
            return get_queen_atk(s, occupied);

        case KING:
            return get_king_atk(s);
    }

    assert(false);
    return 0;
}

void init_movegen(int seed = 38921083);

vector<Move> get_legal_moves(Pos& pos);

void add_pawn_moves(vector<Move>& moves, Pos& pos);
void add_knight_moves(vector<Move>& moves, Pos& pos);
void add_bishop_moves(vector<Move>& moves, Pos& pos);
void add_rook_moves(vector<Move>& moves, Pos& pos);
void add_queen_moves(vector<Move>& moves, Pos& pos);
void add_king_moves(vector<Move>& moves, Pos& pos);

