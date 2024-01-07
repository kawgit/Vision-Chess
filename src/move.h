#pragma once

#include "types.h"
#include "util.h"

inline Move make_move(Move from, Move to, MoveFlag flags) { return (flags << 12) | (from << 6) | to; }

namespace move {

    inline MoveFlag flags(Move move)           { return move >> 12; }
    
    inline bool is_capture(Move move)          { return flags(move) & CAPTURE; }
    inline bool is_double_pawn_push(Move move) { return flags(move) == DOUBLE_PAWN_PUSH; }
    inline bool is_ep(Move move)               { return flags(move) == EP; }
    inline bool is_king_castle(Move move)      { return flags(move) == KING_CASTLE; }
    inline bool is_promotion(Move move)        { return flags(move) & N_PROM; }
    inline bool is_queen_castle(Move move)     { return flags(move) == QUEEN_CASTLE; }

    inline Square to_square(Move move)         { return move & 0b111111; }
    inline Square from_square(Move move)       { return (move >> 6) & 0b111111; }
    inline Piece promotion_piece(Move move)    { return (flags(move) & 0b0011) + KNIGHT; }
    
    inline Square capture_square(Move move) {
        Square from_square_ = from_square(move);
        Square to_square_   = to_square(move);
        return is_ep(move) ? square_of(rank_of(from_square_), file_of(to_square_)) : to_square_;
    }
}