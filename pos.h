#pragma once

#include "types.h"
#include <vector>

using namespace std;

class Pos {
public:
    Pos(string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    BB& getPieceMask(Color c, Piece p) { return piece_masks[c][p]; }

    int getPieceAt(Square s) {return pieces[s];};

    SPiece getSPieceAt(Square s);

    void setPiece(Square s, Color c, Piece p);

    void makeMove(Move m);
    void undoMove();


public:

    Color turn = WHITE;

    Piece pieces[64] = {NO_P};
    BB piece_masks[2][6] = {0};

    Square ep = -1;
    CR cr;

    int move_clock = -1;
    int hm_clock = -1;

    BB key;
    
    vector<Move> move_log;
};