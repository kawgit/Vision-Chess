#include "types.h"
#include "pos.h"
#include "util.h"
#include "zobrist.h"
#include "movegen.h"
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

            setPiece(sq, cl, fen_table[c]);
            col++;
        }
    }

    fen = fen.substr(fen.find(' ')+1);
    if (fen.find('w') != string::npos) turn = WHITE;
    else turn = BLACK;

    notturn = (turn == WHITE ? BLACK : WHITE);

    fen = fen.substr(fen.find(' ')+1);
    if (fen.find('K') != string::npos) cr.adWK();
    if (fen.find('Q') != string::npos) cr.adWQ();
    if (fen.find('k') != string::npos) cr.adBK();
    if (fen.find('q') != string::npos) cr.adBQ();
    fen = fen.substr(fen.find(' ')+1);
    if (fen.at(0) != '-') setEP(RC2SQ(fen.at(1)-'1', fen.at(0)-'a'));
    fen = fen.substr(fen.find(' ')+1);
    if (fen == "" || fen.at(0) == ' ') goto out;
    hm_clock = stoi(fen.substr(0, fen.find(' ')));
    fen = fen.substr(fen.find(' ')+1);
    if (fen == "" || fen.at(0) == ' ') goto out;
    move_clock = stoi(fen);

    out:

    key = _hash(*this);

    move_log.reserve(150);
    cr_log.reserve(150);
    ep_log.reserve(150);
    key_log.reserve(150);
}

SPiece Pos::getSPieceAt(Square s) {
    int p = getPieceAt(s, WHITE);
    if (p == NO_P) p = getPieceAt(s, BLACK);
    return SPiece(((p != NO_P && bitAt(pieces[WHITE][p], s)) ? WHITE : BLACK), p);
}

int Pos::getPieceAt(Square s, Color c) {
    BB a = getBB(s);
    if (pieces[c][PAWN] & a) return PAWN;
    if (pieces[c][KNIGHT] & a) return KNIGHT;
    if (pieces[c][BISHOP] & a) return BISHOP;
    if (pieces[c][ROOK] & a) return ROOK;
    if (pieces[c][QUEEN] & a) return QUEEN;
    if (pieces[c][KING] & a) return KING;
    return NO_P;
}

__forceinline void Pos::setPiece(Square s, Color c, Piece p) {
    key ^= (c== WHITE ? z_white_squares[p][s] : z_black_squares[p][s]);
    getPieceMask(c, p) |= getBB(s);
}

__forceinline void Pos::remPiece(Square s, Color c, Piece p) {
    key ^= (c== WHITE ? z_white_squares[p][s] : z_black_squares[p][s]);
    getPieceMask(c, p) &= ~getBB(s);
}

__forceinline void Pos::remCR(CR_ CR) {
    cr.rmCR(CR);
    key ^= z_cr[CR];
}

__forceinline void Pos::setEP(Square s) {
    if (ep < 64) key ^= z_ep[ep%8];
    ep = s;
    if (ep < 64) key ^= z_ep[ep%8];
}

__forceinline void Pos::switchTurn() {
    turn = notturn;
    notturn = (turn == WHITE ? BLACK : WHITE);
    key ^= z_turn;
}

void Pos::makeMove(Move &m) {
    move_log.push_back(m);
    cr_log.push_back(cr);
    ep_log.push_back(ep);
    key_log.push_back(key);

    int fr = m.getFr();
    int to = m.getTo();
    int fp = m.getFp();
    int tp = m.getTp();

    remPiece(fr, turn, fp);

    setEP(-1);

    if (m.getFlags()) {

        if (m.isCapture()) {
            if (m.isEp())
                remPiece(to - (turn == WHITE ? 8 : -8), notturn, PAWN);
            else
                remPiece(to, notturn, tp);
        }
        else if (m.isDoublePawnPush() && (getKingAtk(to) & rank_masks[to/8] & pieces[notturn][PAWN]))
            setEP(to - (turn == WHITE ? 8 : -8));
        else if (m.isKC()) {
            if (turn == WHITE) {
                remPiece(H1, turn, ROOK);
                setPiece(F1, turn, ROOK);
                remCR(WKS);
            }
            else {
                remPiece(H8, turn, ROOK);
                setPiece(F8, turn, ROOK);
                remCR(BKS);
            }
        }
        else if (m.isQC()) {
            if (turn == WHITE) {
                remPiece(A1, turn, ROOK);
                setPiece(D1, turn, ROOK);
                remCR(WQS);
            }
            else {
                remPiece(A8, turn, ROOK);
                setPiece(D8, turn, ROOK);
                remCR(BQS);
            }
        }
    }

    if (m.isPromotion())
        setPiece(to, turn, m.getProm());
    else 
        setPiece(to, turn, fp);

    if (cr.bits) {
        if (turn == WHITE) {
            if (cr.bits & 0b0011) {
                if (fp == ROOK) {
                    if (cr.getWK() && fr == H1) remCR(WKS);
                    if (cr.getWQ() && fr == A1) remCR(WQS);
                }
                if (fp == KING) {
                    if (cr.getWK()) remCR(WKS);
                    if (cr.getWQ()) remCR(WQS);
                }
            }

            if (cr.bits & 0b1100) {
                if (tp == ROOK) {
                    if (cr.getBK() && to == H8) remCR(BKS);
                    if (cr.getBQ() && to == A8) remCR(BQS);
                }
            }
        }
        else {
            if (cr.bits & 0b1100) {
                if (fp == ROOK) {
                    if (cr.getBK() && fr == H8) remCR(BKS);
                    if (cr.getBQ() && fr == A8) remCR(BQS);
                }
                if (fp == KING) {
                    if (cr.getBK()) remCR(BKS);
                    if (cr.getBQ()) remCR(BQS);
                }
            }

            if (cr.bits & 0b0011) {
                if (tp == ROOK) {
                    if (cr.getWK() && to == H1) remCR(WKS);
                    if (cr.getWQ() && to == A1) remCR(WQS);
                }
            }
        }

    }

    /* do later, need log for hm_clock
    if (turn == BLACK) move_clock++;
    hm_clock++;
    if (fp == PAWN || move.isCapture()) hm_clock = 0;
    */

    switchTurn();
}

void Pos::undoMove() {
    Move m = move_log.back();
    cr = cr_log.back();
    ep = ep_log.back();

    move_log.pop_back();
    cr_log.pop_back();
    ep_log.pop_back();


    switchTurn();

    int fr = m.getFr();
    int to = m.getTo();
    int fp = m.getFp();
    int tp = m.getTp();

    setPiece(fr, turn, fp);

    if (m.isPromotion()) remPiece(to, turn, m.getProm());
    else remPiece(to, turn, fp);

    if (m.getFlags()) {
        if (m.isCapture()) {
            if (m.isEp())
                setPiece(to - (turn == WHITE ? 8 : -8), notturn, PAWN);
            else
                setPiece(to, notturn, tp);
        }
        else if (m.isKC()) {
            if (turn == WHITE) {
                remPiece(F1, turn, ROOK);
                setPiece(H1, turn, ROOK);
            }
            else {
                remPiece(F8, turn, ROOK);
                setPiece(H8, turn, ROOK);
            }
        }
        else if (m.isQC()) {
            if (turn == WHITE) {
                remPiece(D1, turn, ROOK);
                setPiece(A1, turn, ROOK);
            }
            else {
                remPiece(D8, turn, ROOK);
                setPiece(A8, turn, ROOK);
            }
        }
    }
    
    key = key_log.back();
    key_log.pop_back();
}

BB Pos::getAtkMask(Color c) {
    BB mask = 0ULL;

    if (c == WHITE)
        mask |= ((getPieceMask(c, PAWN) & ~file_masks[7]) << 9) | ((getPieceMask(c, PAWN) & ~file_masks[0]) << 7);
    else
        mask |= ((getPieceMask(c, PAWN) & ~file_masks[0]) >> 9) | ((getPieceMask(c, PAWN) & ~file_masks[7]) >> 7);

    BB knights = getPieceMask(c, KNIGHT);
    while (knights) {
        int from = poplsb(knights);
        mask |= getKnightAtk(from);
    }

    BB nonking_occ = occ & ~getPieceMask(getOppositeColor(c), KING);

    BB bishops = getPieceMask(c, BISHOP);
    while (bishops) {
        int from = poplsb(bishops);
        mask |= getBishopAtk(from, nonking_occ);
    }

    BB rooks = getPieceMask(c, ROOK);
    while (rooks) {
        int from = poplsb(rooks);
        mask |= getRookAtk(from, nonking_occ);
    }

    BB queens = getPieceMask(c, QUEEN);
    while (queens) {
        int from = poplsb(queens);
        mask |= getQueenAtk(from, nonking_occ);
    }

    return mask;
}