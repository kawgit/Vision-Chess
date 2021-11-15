#include "types.h"
#include "util.h"

BB square_masks[64] = {};
BB file_masks[8] = {};
BB rank_masks[8] = {};

void initBB() {
    const BB FILE = 0b0000000100000001000000010000000100000001000000010000000100000001;
    const BB RANK = 0b0000000000000000000000000000000000000000000000000000000011111111;

    for (int i = 0; i != 64; i++) {
        square_masks[i] = 1ULL<<i;
    }

    for (int i = 0; i != 8; i++) {
        file_masks[i] = FILE<<i;
    }

    for (int i = 0; i != 8; i++) {
        rank_masks[i] = RANK<<i*8;
    }
}

string pnot[6] = {"p","n","b","r","q","k"};
string Move::getSAN() {
    return getSquareN(getFr()) + getSquareN(getTo()) + (isPromotion() ? pnot[getProm()] : "");
}

Color getOppositeColor(Color c) {
    return (c == WHITE ? BLACK : WHITE);
}

void order(vector<Move> &moveList, Move table_move, Move huerist_move) {
    vector<Move> save = moveList;
    moveList.clear();
    moveList.reserve(save.size());
    for (Move &m : save) {
        if (m.isPromotion()) m.eval += 10;
        if (m.isCapture()) m.eval += 5 + m.getTp() - m.getFp();
        if (m.encode & 0b111111111111 == table_move.encode & 0b111111111111) m.eval += 40;
        if (m.encode & 0b111111111111 == huerist_move.encode & 0b111111111111) m.eval += 35;
        insertToSortedMoveList(moveList, m);
    }
}

void sort(vector<Move> &moveList) {
    vector<Move> save = moveList;
    moveList.clear();
    moveList.reserve(save.size());
    for (Move &m : save) {
        insertToSortedMoveList(moveList, m);
    }
}

void insertToSortedMoveList(vector<Move> &moveList, Move m) {
    moveList.push_back(m);
    for (int i = moveList.size()-2; i != -1; i--) {
        if (moveList[i].eval < m.eval) {
            moveList[i+1] = moveList[i];
        }
        else {
            moveList[i+1] = m;
            return;
        }
    }
    moveList[0] = m;
}