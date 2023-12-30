#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

typedef uint8_t  Square;
typedef uint8_t  File;
typedef uint8_t  Rank;
typedef uint64_t BB;

typedef uint8_t  Color;
typedef uint8_t  Piece;

typedef int8_t   Depth;
typedef uint8_t  Clock;
typedef uint8_t  Direction;
typedef uint8_t  CR_Flag;
typedef uint8_t  CR_Index;

typedef int      Score;
typedef int16_t  Eval;

enum Squares    : Square    { A1, B1, C1, D1, E1, F1, G1, H1, A2, B2, C2, D2, E2, F2, G2, H2, A3, B3, C3, D3, E3, F3, G3, H3, A4, B4, C4, D4, E4, F4, G4, H4, A5, B5, C5, D5, E5, F5, G5, H5, A6, B6, C6, D6, E6, F6, G6, H6, A7, B7, C7, D7, E7, F7, G7, H7, A8, B8, C8, D8, E8, F8, G8, H8, SQUARE_NONE = 255 };
enum Files      : File      { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, EP_NONE };
enum Ranks      : Rank      { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };
enum BBs        : BB        { BB_ZERO = 0ULL, BB_FULL = ~0ULL };

enum Colors     : Color     { COLOR_NONE, BLACK, WHITE };
enum Pieces     : Piece     { PIECE_NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

enum Depths     : Depth     { DEPTH_MAX = 127 };
enum Clocks     : Clock     { CLOCK_MAX = DEPTH_MAX };
enum Directions : Direction { NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST };
enum CR_Flags   : CR_Flag   { NORIGHTS, WKS_F, WQS_F, BKS_F=4, BQS_F=8, FULLRIGHTS=15 };
enum CR_Indexs  : CR_Index  { WKS_I, WQS_I, BKS_I, BQS_I };

enum Scores     : Score     { SCORE_MAX = (1ULL << 31) - 1 };
enum Evals      : Eval      { INF = 32767, MINMATE = 32767 - DEPTH_MAX };

inline bool getWK(CR_Flag cr) { return cr & WKS_F; };
inline bool getWQ(CR_Flag cr) { return cr & WQS_F; };
inline bool getBK(CR_Flag cr) { return cr & BKS_F; };
inline bool getBQ(CR_Flag cr) { return cr & BQS_F; };

inline Color opp(Color c) { return c == WHITE ? BLACK : WHITE; }

inline int rc(int r, int c) {
	return r*8+c;
}

inline Rank rank_of(Square sq) {
    return sq / 8;
}

inline File file_of(Square sq) {
    return sq % 8;
}

inline string eval_to_string(Eval eval) {
    return (abs(eval) >= MINMATE ? ((eval > 0 ? "mate " : "mate -") + to_string(INF-abs(eval))) : ("cp " + to_string(eval)));
}

inline string square_to_string(Square s) {
    if (s == SQUARE_NONE) return "-";
    string r = "";
    r += (s%8 + 'a');
    r += (s/8 + '1');
    return r;
}

inline Square string_to_square(string notation) {
    return rc(notation[1]-'1', notation[0]-'a');
}