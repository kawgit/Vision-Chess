#pragma once

#include <array>
#include <cassert>

#include "bits.h"
#include "types.h"
#include "util.h"



namespace attacks {

    const size_t MAGIC_BITS = 14;
    const size_t MAGIC_ENTRY_SIZE = 1ULL << MAGIC_BITS;
    const size_t MAGIC_SHIFT = N_SQUARES - MAGIC_BITS;

    constexpr std::array<std::array<BB, N_SQUARES>, N_COLORS> pawn_atks = []() constexpr {

        std::array<std::array<BB, N_SQUARES>, N_COLORS> result { BB_EMPTY };
        
        for (Square square = A1; square <= H8; square++) {
            result[WHITE][square] = shift<NORTH>(shift<WEST>(bb_of(square)) | shift<EAST>(bb_of(square)));
            result[BLACK][square] = shift<SOUTH>(shift<WEST>(bb_of(square)) | shift<EAST>(bb_of(square)));
        }

        return result;
        
    }();

    constexpr std::array<BB, N_SQUARES> knight_atks = []() constexpr {

        std::array<BB, N_SQUARES> result { BB_EMPTY };

        for (Square square = A1; square <= H8; square++) {
            
            const Rank rank = rank_of(square);
            const File file = file_of(square);

            for (size_t direction = 0; direction < 8; direction++) {
                
                const Rank target_rank = rank + KNIGHT_OFFSETS[direction][1];
                const File target_file = file + KNIGHT_OFFSETS[direction][0];

                if (target_rank < RANK_1 || target_rank > RANK_8 || target_file < FILE_A || target_file > FILE_H)
                    continue;

                const Square target_square = square_of(target_rank, target_file);

                result[square] |= bb_of(target_square);

            }
        }

        return result;

    }();

    constexpr std::array<BB, N_SQUARES> bishop_atks = []() constexpr {

        std::array<BB, N_SQUARES> result { BB_EMPTY };

        for (Square square = A1; square <= H8; square++) {
            
            result[square] = bb_gun(square, NORTHEAST)
                           | bb_gun(square, SOUTHEAST)
                           | bb_gun(square, SOUTHWEST)
                           | bb_gun(square, NORTHWEST);

        }

        return result;

    }();

    constexpr std::array<BB, N_SQUARES> rook_atks = []() constexpr {

        std::array<BB, N_SQUARES> result { BB_EMPTY };

        for (Square square = A1; square <= H8; square++) {
            
            result[square] = bb_gun(square, NORTH)
                           | bb_gun(square, EAST)
                           | bb_gun(square, SOUTH)
                           | bb_gun(square, WEST);

        }

        return result;

    }();

    constexpr std::array<BB, N_SQUARES> queen_atks = []() constexpr {

        std::array<BB, N_SQUARES> result { BB_EMPTY };

        for (Square square = A1; square <= H8; square++) {
            
            result[square] = bb_gun(square, NORTH)
                           | bb_gun(square, NORTHEAST)
                           | bb_gun(square, EAST)
                           | bb_gun(square, SOUTHEAST)
                           | bb_gun(square, SOUTH)
                           | bb_gun(square, SOUTHWEST)
                           | bb_gun(square, WEST)
                           | bb_gun(square, NORTHWEST);

        }

        return result;

    }();

    constexpr std::array<BB, N_SQUARES> king_atks = []() constexpr {

        std::array<BB, N_SQUARES> result { BB_EMPTY };

        for (Square square = A1; square <= H8; square++) {
            
            result[square] = shift<NORTH>    (bb_of(square))
                           | shift<NORTHEAST>(bb_of(square))
                           | shift<EAST>     (bb_of(square))
                           | shift<SOUTHEAST>(bb_of(square))
                           | shift<SOUTH>    (bb_of(square))
                           | shift<SOUTHWEST>(bb_of(square))
                           | shift<WEST>     (bb_of(square))
                           | shift<NORTHWEST>(bb_of(square));

        }

        return result;

    }();

    extern BB bishop_blockermasks[N_SQUARES];
    extern BB rook_blockermasks  [N_SQUARES];

    extern BB bishop_magics [N_SQUARES];
    extern BB rook_magics   [N_SQUARES];
    
    extern std::array<std::array<BB, MAGIC_ENTRY_SIZE>, N_SQUARES> bishop_table;
    extern std::array<std::array<BB, MAGIC_ENTRY_SIZE>, N_SQUARES> rook_table;

    inline constexpr BB pawn(Square square, Color color) { 
        assert(is_okay_square(square));
        assert(is_okay_color(color));
        return pawn_atks[color][square];
    }

    inline constexpr BB knight(Square square) {
        assert(is_okay_square(square));
        return knight_atks[square];
    }

    inline constexpr BB bishop(Square square) {
        assert(is_okay_square(square));
        return bishop_atks[square];
    }

    inline constexpr BB rook(Square square) {
        assert(is_okay_square(square));
        return rook_atks[square];
    }

    inline constexpr BB queen(Square square) {
        assert(is_okay_square(square));
        return queen_atks[square];
    }

    inline constexpr BB king(Square square) {
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

    inline constexpr BB pawns(BB squares, Color color) { 

        assert(is_okay_color(color));

        squares = shift<WEST>(squares) | shift<EAST>(squares);

        return color == WHITE ? shift<NORTH>(squares) : shift<SOUTH>(squares);
    }

    inline constexpr BB knights(BB squares) {

        BB result = BB_EMPTY;

        while (squares) {
            const Square square = poplsb(squares);
            result |= knight(square);
        }

        return result;
    }

    inline constexpr BB bishops(BB squares, BB occupied) {
        
        BB result = BB_EMPTY;

        while (squares) {
            const Square square = poplsb(squares);
            result |= bishop(square, occupied);
        }

        return result;
    }

    inline constexpr BB rooks(BB squares, BB occupied) {
        
        BB result = BB_EMPTY;

        while (squares) {
            const Square square = poplsb(squares);
            result |= rook(square, occupied);
        }

        return result;
    }

    inline constexpr BB queens(BB squares, BB occupied) {
        
        BB result = BB_EMPTY;

        while (squares) {
            const Square square = poplsb(squares);
            result |= bishop(square, occupied) | rook(square, occupied);
        }

        return result;
    }

    inline constexpr BB kings(BB squares) {
        
        BB result = BB_EMPTY;

        while (squares) {
            const Square square = poplsb(squares);
            result |= king(square);
        }

        return result;
    }

    inline BB lookup(const Piece piece, const Square square, const BB occupied, const Color color) { 
        
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

    inline BB lookup(const Piece piece, const Square square, const BB occupied) { 
        
        assert(is_okay_square(square));
        assert(is_okay_piece(piece));
        assert(piece != PAWN);

        switch (piece) {
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

    inline BB lookup(const Piece piece, const Square square) { 
        
        assert(is_okay_square(square));
        assert(is_okay_piece(piece));
        assert(piece != PAWN);

        switch (piece) {
            case KNIGHT:
                return attacks::knight(square);
            case BISHOP:
                return attacks::bishop(square);
            case ROOK:
                return attacks::rook(square);
            case QUEEN:
                return attacks::queen(square);
            case KING:
                return attacks::king(square);
        }

        assert(false);

        return 0;

    }

    void init();
}