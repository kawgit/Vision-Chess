#pragma once

#include <cstdlib>
#include <stdint.h>
#include <string>
#include <iostream>
#include <vector>
#include "bits.h"

using namespace std;

typedef int16_t Eval;

const Eval INF = 32767;
const Eval MINMATE = INF - 128;

typedef uint32_t Score;
const Score SCORE_MAX = (1ULL << 32) - 1;

inline string eval_to_string(Eval eval) {
    return (abs(eval) >= MINMATE ? ((eval > 0 ? "mate " : "mate -") + to_string(INF-abs(eval))) : ("cp " + to_string(eval)));
}

typedef int8_t Depth;
const Depth DEPTHMAX = 127;

typedef uint8_t Clock;
typedef uint8_t Square;
typedef uint8_t File;
typedef uint8_t Rank;

enum Files : File { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, EP_NONE};
enum Ranks : Rank { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};

enum Squares : Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQUARE_NONE = 255
};

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

typedef uint8_t Color;
enum Colors : Color {COLOR_NONE, BLACK, WHITE};
inline Color opp(Color c) { return c == WHITE ? BLACK : WHITE; }

typedef uint8_t Piece;
enum Pieces : Piece {PIECE_NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

typedef uint16_t Move;
typedef uint8_t MoveFlag;

enum Moves : Move {MOVE_NONE = 0, MOVE_NULL = 0xFFFF};

inline Move make_move(Move from, Move to, MoveFlag flags) {
    return (flags << 12) | (from << 6) | to;
}

enum MoveFlags : MoveFlag {QUIET=0, DOUBLE_PAWN_PUSH, KING_CASTLE, QUEEN_CASTLE, CAPTURE, EP, N_PROM=8, B_PROM, R_PROM, Q_PROM, N_PROM_CAPTURE, B_PROM_CAPTURE, R_PROM_CAPTURE, Q_PROM_CAPTURE};

inline Square get_to(Move m) { return m & 0b111111; }
inline Square get_from(Move m) { return (m >> 6) & 0b111111; }
inline MoveFlag get_flags(Move m) { return m >> 12; }

inline bool is_double_pawn_push(Move m) { return get_flags(m) == DOUBLE_PAWN_PUSH; }
inline bool is_king_castle(Move m) { return get_flags(m) == KING_CASTLE; }
inline bool is_queen_castle(Move m) { return get_flags(m) == QUEEN_CASTLE; }
inline bool is_capture(Move m) { return get_flags(m) & CAPTURE; }
inline bool is_ep(Move m) { return get_flags(m) == EP; }
inline bool is_promotion(Move m) { return get_flags(m) & N_PROM; }
inline Piece get_promotion_type(Move m) { return (get_flags(m) & 0b0011) + KNIGHT; }

string getSAN(Move m);

string to_string(vector<Move> moves);
void print(vector<Move> moves);

//CASTLE RIGHTS

typedef uint8_t CR;
enum CR_flag : CR {NORIGHTS, WKS_F, WQS_F, BKS_F=4, BQS_F=8, FULLRIGHTS=15};
enum CR_Index : CR {WKS_I, WQS_I, BKS_I, BQS_I};

inline bool getWK(CR& cr) { return cr & WKS_F; };
inline bool getWQ(CR& cr) { return cr & WQS_F; };
inline bool getBK(CR& cr) { return cr & BKS_F; };
inline bool getBQ(CR& cr) { return cr & BQS_F; };

//DIRECTIONS

enum RAY_DIRECTIONS : uint8_t {NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST};