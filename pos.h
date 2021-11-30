#pragma once

#include "types.h"
#include <vector>

using namespace std;

class Pos {
public:
    Pos(string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    inline BB& getPieceMask(Color c, Piece p) { return pieces[c][p]; }

    inline BB getOcc(Color c) { return (pieces[c][PAWN] | pieces[c][KNIGHT] | pieces[c][BISHOP] | pieces[c][ROOK] | pieces[c][QUEEN] | pieces[c][KING]); }
    inline BB getOcc() { return getOcc(WHITE) | getOcc(BLACK); }

    int getPieceAt(Square s, Color c);

    SPiece getSPieceAt(Square s);

    void setPiece(Square s, Color c, Piece p);
    void remPiece(Square s, Color c, Piece p);
    void remCR(CR_ CR);
    void setEP(Square s);
    void switchTurn();
    void makeNullMove();
    void undoNullMove();

    void makeMove(Move &m);
    void undoMove();
    
    void makeMoveSAN(string SAN);
    string getPGN();

    BB getAtkMask(Color c);

    inline void updateMasks() {
        turn_occ = getOcc(turn);
        notturn_occ = getOcc(notturn);
        occ = turn_occ | notturn_occ;
    }

public:

    Color turn = WHITE;
    Color notturn = BLACK;

    BB pieces[2][6] = {0};

    Square ep = -1;
    CR cr;

    int move_clock = -1;
    int hm_clock = -1;

    BB key;
    
    vector<Move> move_log;
    vector<Square> ep_log;
    vector<CR> cr_log;
    vector<BB> key_log;

    //masks

    BB turn_occ = 0;
    BB notturn_occ = 0;
    BB occ = 0;

    bool inCheck = false;
    int nullMovesMade = 0;
};