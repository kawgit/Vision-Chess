#pragma once

#include <cassert>

#include "bits.h"
#include "types.h"
#include "util.h"

using namespace std;

namespace attacks {

    const size_t MAGIC_BITS = 14;
    const size_t MAGIC_ENTRY_SIZE = 1ULL << MAGIC_BITS;
    const size_t MAGIC_SHIFT = N_SQUARES - MAGIC_BITS;

    extern BB rays          [N_SQUARES][N_DIRECTIONS];

    extern BB pawn_atks     [N_COLORS][N_SQUARES];
    extern BB knight_atks             [N_SQUARES];
    extern BB bishop_atks             [N_SQUARES];
    extern BB rook_atks               [N_SQUARES];
    extern BB queen_atks              [N_SQUARES];
    extern BB king_atks               [N_SQUARES];

    extern BB bishop_blockermasks[N_SQUARES];
    extern BB rook_blockermasks  [N_SQUARES];

    extern BB bishop_magics [N_SQUARES];
    extern BB rook_magics   [N_SQUARES];
    
    extern array<array<BB, MAGIC_ENTRY_SIZE>, N_SQUARES> bishop_table;
    extern array<array<BB, MAGIC_ENTRY_SIZE>, N_SQUARES> rook_table;

    inline BB pawn(Square square, Color color) { 
        assert(is_okay_square(square));
        assert(is_okay_color(color));
        return pawn_atks[color][square];
    }

    inline BB knight(Square square) {
        assert(is_okay_square(square));
        return knight_atks[square];
    }

    inline BB bishop(Square square) {
        assert(is_okay_square(square));
        return bishop_atks[square];
    }

    inline BB rook(Square square) {
        assert(is_okay_square(square));
        return rook_atks[square];
    }

    inline BB queen(Square square) {
        assert(is_okay_square(square));
        return queen_atks[square];
    }

    inline BB king(Square square) {
        assert(is_okay_square(square));
        return king_atks[square];
    }
    
    inline BB bishop(Square square, BB occupied) {
        assert(is_okay_square(square));
        return bishop_table[square][(bishop_magics[square] * (occupied & bishop_blockermasks[square])) >> MAGIC_SHIFT];
    }

    inline BB rook(Square square, BB occupied) {
        assert(is_okay_square(square));
        return rook_table[square][(rook_magics[square] * (occupied & rook_blockermasks[square])) >> MAGIC_SHIFT];
    }

    inline BB queen(Square square, BB occupied) {
        assert(is_okay_square(square));
        return attacks::rook(square, occupied) | attacks::bishop(square, occupied);
    }

    inline BB lookup(const Piece piece, const Square square, const Color color, const BB occupied) { 
        
        assert(is_okay_square(square));
        assert(is_okay_piece(piece));
        assert(is_okay_color(color));

        switch (piece) {
            case PAWN:
                return attacks::pawn(square, color);
            case KNIGHT:
                return attacks::knight(square);
            case BISHOP:
                return attacks::bishop(square, occupied);
            case ROOK:
                return attacks::rook(square, occupied);
            case QUEEN:
                return attacks::queen(square, occupied);
            case KING:
                return attacks::king(square);
        }

        assert(false);

        return 0;

    }

    void init();
}