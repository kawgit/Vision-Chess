#include "types.h"
#include "pos.h"
#include "util.h"
#include "zobrist.h"
#include <string>
#include <map>

#include <iostream>

using namespace std;

map<char, Piece> fen_table {
    {'P', PAWN},
    {'N', KNIGHT},
    {'B', BISHOP},
    {'R', ROOK},
    {'Q', QUEEN},
    {'K', KING},
};

Pos::Pos(string fen) {
    string board = fen.substr(0, fen.find(' '));
    int col = 0;
    int row = 7;
    for (char c : board) {
        int a = c - '0';
        if (c == '/') {
            row--;
            col = 0;
        }
        else if (a >= 1 && a <= 8) col+=a;
        else {
            int sq = RC2SQ(row, col);
            Color cl = WHITE;
            if (c >= 'a') {
                cl = BLACK;
                c = toupper(c);
            }

            setPiece(cl, fen_table[c], sq);
            col++;
        }
    }

    fen = fen.substr(fen.find(' ')+1);
    if (fen.find('w') != string::npos) turn = WHITE;
    else turn = BLACK;
    fen = fen.substr(fen.find(' ')+1);
    if (fen.find('K') != string::npos) cr.adWK();
    if (fen.find('Q') != string::npos) cr.adWQ();
    if (fen.find('k') != string::npos) cr.adBK();
    if (fen.find('q') != string::npos) cr.adBQ();
    fen = fen.substr(fen.find(' ')+1);
    if (fen.at(0) != '-') RC2SQ(fen.at(1), fen.at(0)-'a');
    fen = fen.substr(fen.find(' ')+1);
    hm_clock = stoi(fen.substr(0, fen.find(' ')));
    fen = fen.substr(fen.find(' ')+1);
    move_clock = stoi(fen);

    key = _hash(*this);
}

SPiece Pos::getSPieceAt(Square s) {
    BB sm = getBB(s);
    for (int i = 0; i != 6; i++) {
        if (white_pieces[i] & sm) return SPiece(WHITE, i);
        if (black_pieces[i] & sm) return SPiece(BLACK, i);
    }
    return SPiece(WHITE, NO_P);
}