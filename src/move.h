#pragma once

#include <string>
#include <cassert>

#include "types.h"

using namespace std;

typedef uint16_t Move;
typedef uint8_t MoveFlag;

enum Moves : Move {MOVE_NONE = 0, MOVE_NULL = 0xFFFF};
enum MoveFlags : MoveFlag {QUIET=0, DOUBLE_PAWN_PUSH, KING_CASTLE, QUEEN_CASTLE, CAPTURE, EP, N_PROM=8, B_PROM, R_PROM, Q_PROM, N_PROM_CAPTURE, B_PROM_CAPTURE, R_PROM_CAPTURE, Q_PROM_CAPTURE};

inline Move make_move(Move from, Move to, MoveFlag flags) { return (flags << 12) | (from << 6) | to; }

inline Square get_to(Move m)            { return m & 0b111111; }
inline Square get_from(Move m)          { return (m >> 6) & 0b111111; }
inline MoveFlag get_flags(Move m)       { return m >> 12; }

inline bool is_double_pawn_push(Move m) { return get_flags(m) == DOUBLE_PAWN_PUSH; }
inline bool is_king_castle(Move m)      { return get_flags(m) == KING_CASTLE; }
inline bool is_queen_castle(Move m)     { return get_flags(m) == QUEEN_CASTLE; }
inline bool is_capture(Move m)          { return get_flags(m) & CAPTURE; }
inline bool is_ep(Move m)               { return get_flags(m) == EP; }
inline bool is_promotion(Move m)        { return get_flags(m) & N_PROM; }
inline Piece get_promotion_type(Move m) { return (get_flags(m) & 0b0011) + KNIGHT; }

string to_san(Move m);
string to_string(vector<Move> moves);
void print(vector<Move> moves);