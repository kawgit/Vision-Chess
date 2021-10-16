#pragma once

#include "types.h"

using namespace std;

class Pos {
public:
    Pos(string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    inline void setPiece(Color c, Piece p, Square s) { (c == WHITE ? white_pieces[p] : black_pieces[p]) |= getBB(s);}

    inline BB* getPieces(Color c) { return c == WHITE ? white_pieces : black_pieces; }
    inline BB& getPieces(Color c, Piece p) { return c == WHITE ? white_pieces[p] : black_pieces[p]; }

    SPiece getSPieceAt(Square s);

public:

    Color turn = WHITE;
    BB white_pieces[6] = {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL};
    BB black_pieces[6] = {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL};

    Square ep = -1;
    CR cr;

    int move_clock = -1;
    int hm_clock = -1;

    BB key;
};