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

const BB m1  = 0x5555555555555555; //binary: 0101...
const BB m2  = 0x3333333333333333; //binary: 00110011..
const BB m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
const BB h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

int bitcount(BB x)
{
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    return (x * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}