#pragma once

#include <stdint.h>
#include <string>
#include <iostream>
#include <vector>
#include "bits.h"

using namespace std;

typedef int16_t Eval;

const Eval INF = 32767;
const Eval MINMATE = INF - 128;

inline string evalToString(Eval& eval) {
    return (abs(eval) >= MINMATE ? ((eval > 0 ? "mate " : "mate -") + to_string(INF-abs(eval) + 1)) : ("cp " + to_string(eval)));
}

typedef int8_t Depth;
typedef uint8_t Clock;

//SQUARE
typedef uint8_t Square;

enum Squares : Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQUARENONE = 255
};

inline string sqToNot(Square s) {
    if (s == SQUARENONE) return "-";
    string r = "";
    r += (s%8 + 'a');
    r += (s/8 + '1');
    return r;
}

inline Square notToSq(string notation) {
    return rc(notation[1]-'1', notation[0]-'a');
}

//COLOR
typedef bool Color;
enum Colors : Color {BLACK, WHITE};
inline Color getOppositeColor(Color c) { return c == WHITE ? BLACK : WHITE; }

//PIECE
typedef uint8_t Piece;
enum Pieces : Piece {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECENONE};

//MOVE
typedef uint16_t Move;
typedef uint8_t MoveFlag;

enum Moves : Move {MOVENONE = 0xFFFF};

inline Move constructMove(Move from, Move to, MoveFlag flags) {
    return (flags << 12) | (from << 6) | to;
}

enum MoveFlags : MoveFlag {QUIET=0, DOUBLE_PAWN_PUSH, KINGCASTLE, QUEENCASTLE, CAPTURE, EP, N_PROM=8, B_PROM, R_PROM, Q_PROM, N_PROM_CAPTURE, B_PROM_CAPTURE, R_PROM_CAPTURE, Q_PROM_CAPTURE};

inline Square getTo(Move m) { return m & 0b111111; }
inline Square getFrom(Move m) { return (m >> 6) & 0b111111; }
inline MoveFlag getFlags(Move m) { return m >> 12; }

inline bool isDoublePawnPush(Move m) { return getFlags(m) == DOUBLE_PAWN_PUSH; }
inline bool isKingCastle(Move m) { return getFlags(m) == KINGCASTLE; }
inline bool isQueenCastle(Move m) { return getFlags(m) == QUEENCASTLE; }
inline bool isCapture(Move m) { return getFlags(m) & CAPTURE; }
inline bool isEp(Move m) { return getFlags(m) == EP; }
inline bool isPromotion(Move m) { return getFlags(m) & N_PROM; }
inline Piece getPromotionType(Move m) { return (getFlags(m) & 0b0011) + KNIGHT; }

string getSAN(Move m);

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