#pragma once

#include <stdint.h>
#include <string>

using namespace std;

typedef uint64_t BB; //bitboard
typedef uint8_t Square;
typedef uint8_t Piece;
typedef bool Color;

struct CR { //castle rights
    uint8_t bits = 0;

    inline void setFull() { bits = 0b1111;}

    inline void adWK() { bits |= 0b0001;}
    inline void adWQ() { bits |= 0b0010;}
    inline void adBK() { bits |= 0b0100;}
    inline void adBQ() { bits |= 0b1000;}

    inline void rmWK() { bits &= ~0b0001;}
    inline void rmWQ() { bits &= ~0b0010;}
    inline void rmBK() { bits &= ~0b0100;}
    inline void rmBQ() { bits &= ~0b1000;}

    inline bool getWK() { return bits & 0b0001;}
    inline bool getWQ() { return bits & 0b0010;}
    inline bool getBK() { return bits & 0b0100;}
    inline bool getBQ() { return bits & 0b1000;}

    inline void operator=(CR a) { bits = a.bits;}
    inline bool operator==(CR a) { return bits == a.bits; }
    inline bool operator!=(CR a) { return bits != a.bits; }
};

//enums
enum Colors : Color {BLACK = 0, WHITE = 1};
enum Pieces : Piece {PAWN = 0, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_P};
enum Squares : Square {
    A1 = 0, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
};

struct SPiece { //specific piece
    Color c;
    Piece p;
    SPiece(Color C, Piece P) {
        c = C;
        p = P;
    }
};


//BB MASKS
extern BB square_masks[64];
extern BB file_masks[8];
extern BB rank_masks[8];

//init BB masks
void initBB();

inline BB getBB(int s) { return square_masks[s]; }

inline BB getRankBB(int r) { return rank_masks[r]; }

inline BB getFileBB(int f) { return file_masks[f]; }