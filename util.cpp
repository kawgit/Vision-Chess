#include "types.h"
#include "pos.h"
#include "util.h"
#include <iostream>
#include <stdio.h>
#include <inttypes.h>
#include <string>

using namespace std;

const string white_piece_not[7] = {"P", "N", "B", "R", "Q", "K", " "};
const string black_piece_not[7] = {"p", "n", "b", "r", "q", "k", " "};
void print(Pos p) {
    cout<<"  +---+---+---+---+---+---+---+---+"<<endl;
    for (int r = 7; r != -1; r--) {
        cout<<to_string(r+1)<<" | ";
        for (int c = 0; c != 8; c++) {
            SPiece sp = p.getSPieceAt(RC2SQ(r, c));
            cout<<(sp.c == WHITE ? white_piece_not[sp.p] : black_piece_not[sp.p])<<" | ";
        }
        cout<<endl<<"  +---+---+---+---+---+---+---+---+"<<endl;
    }
    cout<<"    a   b   c   d   e   f   g   h  "<<endl;
    printf("Key: %#018llx\n", p.key);
    cout<<"Castle Rights: "
        <<(p.cr.getWK() ? "K" : "")
        <<(p.cr.getWQ() ? "Q" : "")
        <<(p.cr.getBK() ? "k" : "")
        <<(p.cr.getBQ() ? "q" : "")
        <<endl;
    cout<<"En Passant: "<<getSquareN(p.ep)<<endl;
    cout<<"Hm Clock: "<<p.hm_clock<<endl;
    cout<<"Move Clock: "<<p.move_clock<<endl;
    
}

void print(BB b) {
    for (int r = 7; r != -1; r--) {
        for (int c = 0; c != 8; c++) {
            cout<<(bitAt(b, RC2SQ(r, c)) ? "X " : "- ");
        }
        cout<<endl;
    }
}

string getSquareN(Square s) {
    return s < 64 ? string(1, (char)((s%8) + 'A')) + to_string(s/8+1) : "--";
}

int lsb(BB n) { 
    return __builtin_ffsll(n)-1;
};

int poplsb(BB &n) { 
    int i = __builtin_ffsll(n)-1;
    n &= ~(1ULL<<i);
    return i;
};