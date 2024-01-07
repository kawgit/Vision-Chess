#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <vector>
#include <cassert>

using namespace std;

typedef int8_t  Square;
typedef int8_t   File;
typedef int8_t   Rank;
typedef uint64_t BB;

typedef uint8_t  Piece;
typedef uint8_t  Spiece; // Specific piece - color and piece type

typedef int8_t   Depth;
typedef uint8_t  Clock;
typedef uint8_t  Direction;

typedef int      Score;
typedef int16_t  Eval;

typedef uint16_t Move;
typedef uint8_t MoveFlag;

enum Squares     : Square     { A1, B1, C1, D1, E1, F1, G1, H1, A2, B2, C2, D2, E2, F2, G2, H2, A3, B3, C3, D3, E3, F3, G3, H3, A4, B4, C4, D4, E4, F4, G4, H4, A5, B5, C5, D5, E5, F5, G5, H5, A6, B6, C6, D6, E6, F6, G6, H6, A7, B7, C7, D7, E7, F7, G7, H7, A8, B8, C8, D8, E8, F8, G8, H8, N_SQUARES, SQUARE_NONE = N_SQUARES };
enum Files       : File       { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, N_FILES, FILE_NONE = N_FILES };
enum Ranks       : Rank       { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, N_RANKS, RANK_NONE = N_RANKS };
enum Color       : uint8_t    { BLACK, WHITE, N_COLORS, COLOR_NONE = N_COLORS };
enum Pieces      : Piece      { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, N_PIECES, PIECE_NONE = N_PIECES };
enum Directions  : Direction  { NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST, N_DIRECTIONS };

enum Spieces : Spiece { 
    COLOR_MASK = 0b1000, 
    TYPE_MASK  = 0b0111, 
    WHITE_FLAG = 0b1000, // WHITE << 3
    BLACK_FLAG = 0b0000, // BLACK << 3
    WHITE_PAWN   = PAWN   | WHITE_FLAG, 
    WHITE_KNIGHT = KNIGHT | WHITE_FLAG, 
    WHITE_BISHOP = BISHOP | WHITE_FLAG, 
    WHITE_ROOK   = ROOK   | WHITE_FLAG, 
    WHITE_QUEEN  = QUEEN  | WHITE_FLAG, 
    WHITE_KING   = KING   | WHITE_FLAG, 
    BLACK_PAWN   = PAWN   | BLACK_FLAG, 
    BLACK_KNIGHT = KNIGHT | BLACK_FLAG, 
    BLACK_BISHOP = BISHOP | BLACK_FLAG, 
    BLACK_ROOK   = ROOK   | BLACK_FLAG, 
    BLACK_QUEEN  = QUEEN  | BLACK_FLAG, 
    BLACK_KING   = KING   | BLACK_FLAG, 
    SPIECE_NONE,
};

enum BBs        : BB        { BB_EMPTY = 0ULL, BB_FULL = ~0ULL };
enum Depths     : Depth     { DEPTH_MAX = 127 };
enum Clocks     : Clock     { CLOCK_MAX = DEPTH_MAX };

enum Scores     : Score     { SCORE_MAX = (1ULL << 31) - 1 };
enum Evals      : Eval      { INF = 32767, MINMATE = 32767 - DEPTH_MAX };

enum Moves : Move {MOVE_NONE = 0, MOVE_NULL = 0xFFFF};
enum MoveFlags : MoveFlag {QUIET=0, DOUBLE_PAWN_PUSH, KING_CASTLE, QUEEN_CASTLE, CAPTURE, EP, N_PROM=8, B_PROM, R_PROM, Q_PROM, N_PROM_CAPTURE, B_PROM_CAPTURE, R_PROM_CAPTURE, Q_PROM_CAPTURE};

constexpr inline Color operator!(const Color color) {
    return Color(color ^ 1);
}

constexpr inline Color color_of(Spiece piece) {
    return Color((piece & COLOR_MASK) >> 3);
}

constexpr inline Piece type_of(Spiece piece) {
    return Piece(piece & TYPE_MASK);
}

constexpr inline Spiece make_spiece(Color color, Piece piece) {
    return Spiece((color << 3) | (piece));
}

// inline string eval_to_string(Eval eval) {
//     return (abs(eval) >= MINMATE ? ((eval > 0 ? "mate " : "mate -") + to_string(INF-abs(eval))) : ("cp " + to_string(eval)));
// }

// inline string square_to_string(Square s) {
//     if (s == SQUARE_NONE) return "-";
//     string r = "";
//     r += (s%8 + 'a');
//     r += (s/8 + '1');
//     return r;
// }

// inline Square string_to_square(string notation) {
//     return square_of(notation[1]-'1', notation[0]-'a');
// }