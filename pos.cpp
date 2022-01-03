#include "pos.h"
#include "bits.h"
#include "types.h"
#include "movegen.h"
#include "eval.h"
#include <iostream>
#include <map>
#include <string>

using namespace std;

Pos::Pos(string fen) {
	static map<char, Piece> fenmap = {
		{'p', PAWN},
		{'n', KNIGHT},
		{'b', BISHOP},
		{'r', ROOK},
		{'q', QUEEN},
		{'k', KING},
	};

	for (int i = 0; i < 64; i++) {
		mailboxes[WHITE][i] = PIECENONE;
		mailboxes[BLACK][i] = PIECENONE;
	}

	int r = 7;
	int c = 0;
	int i = 0;
	while (!(r == 0 && c == 8)) {
		char ch = fen[i++];
		if (ch == '/') {
			c = 0;
			r--;
		}
		else if (ch >= '0' && ch <= '9') {
			c += ch - '0';
		}
		else {
			Color color = (ch >= 'a' ? BLACK : WHITE);
			setPiece(color, rc(r, c), fenmap[(color == BLACK ? ch : (ch - 'A' + 'a'))]);
			c++;
		}
	}

	fen = fen.substr(i);

	setTurn(fen.find("w") != string::npos ? WHITE : BLACK);

	if (fen.find("K") != string::npos) switchCR(WKS_I);
	if (fen.find("Q") != string::npos) switchCR(WQS_I);
	if (fen.find("k") != string::npos) switchCR(BKS_I);
	if (fen.find("q") != string::npos) switchCR(BQS_I);

	for (int i = 0; i < 3; i++) fen = fen.substr(fen.find(" ")+1);

	if (fen[0] != '-') setEp(notToSq(fen));

	fen = fen.substr(fen.find(" ")+1);
	if (fen.size() == 0) goto end;

	hm_clock = stoi(fen.substr(0, fen.find(" ")));

	fen = fen.substr(fen.find(" ")+1);
	if (fen.size() == 0) goto end;

	m_clock = stoi(fen);

	end:

	move_log.reserve(RESERVE_SIZE);
	hashkey_log.reserve(RESERVE_SIZE);
	cr_log.reserve(RESERVE_SIZE);
	to_piece_log.reserve(RESERVE_SIZE);
	ep_log.reserve(RESERVE_SIZE);

	return;
}

void Pos::makeMove(Move m) {
	move_log.push_back(m);
	hashkey_log.push_back(hashkey);
	cr_log.push_back(cr);
	ep_log.push_back(ep);
	hm_clock_log.push_back(hm_clock);
	repetitions_index_log.push_back(repetitions_index);

	
	Square from = getFrom(m);
	Square to = getTo(m);
	Piece fromPiece = getPieceAt(turn, from);
	Piece toPiece = getPieceAt(notturn, to);
	to_piece_log.push_back(toPiece);

	removePiece(turn, from, fromPiece);

	if (isCapture(m) || fromPiece == PAWN) {
		hm_clock = 0;
		repetitions_index = hashkey_log.size();
	}
	else hm_clock++;
	if (turn == BLACK) m_clock++;

	if (!isPromotion(m)) setPiece(turn, to, fromPiece);
	else setPiece(turn, to, getPromotionType(m));

	setEp(SQUARENONE);

	if (getFlags(m)) {
		if (isCapture(m)) {
			if (!isEp(m)) removePiece(notturn, to, toPiece);
			else removePiece(notturn, to + (turn == WHITE ? -8 : 8), PAWN);
		}
		else if (isDoublePawnPush(m) && (getRowMask(to/8) & getKingAtk(to) & getPieceMask(notturn, PAWN))) 
			setEp(to + (turn == WHITE ? -8 : 8));
		else if (isKingCastle(m)) {
            if (turn == WHITE) {
                removePiece(WHITE, H1, ROOK);
                setPiece(WHITE, F1, ROOK);
            }
            else {
                removePiece(BLACK, H8, ROOK);
                setPiece(BLACK, F8, ROOK);
            }
		}
		else if (isQueenCastle(m)) {
            if (turn == WHITE) {
                removePiece(WHITE, A1, ROOK);
                setPiece(WHITE, D1, ROOK);
            }
            else {
                removePiece(BLACK, A8, ROOK);
                setPiece(BLACK, D8, ROOK);
            }
        }
	}

	if (cr) {
		if (turn == WHITE) {
			if (fromPiece == ROOK) {
				if (getWK(cr) && from == H1) switchCR(WKS_I);
				if (getWQ(cr) && from == A1) switchCR(WQS_I);
			}
			if (fromPiece == KING) {
				if (getWK(cr)) switchCR(WKS_I);
				if (getWQ(cr)) switchCR(WQS_I);
			}
			if (toPiece == ROOK) {
				if (getBK(cr) && to == H8) switchCR(BKS_I);
				if (getBQ(cr) && to == A8) switchCR(BQS_I);
			}
		}
		else {
			if (fromPiece == ROOK) {
				if (getBK(cr) && from == H8) switchCR(BKS_I);
				if (getBQ(cr) && from == A8) switchCR(BQS_I);
			}
			if (fromPiece == KING) {
				if (getBK(cr)) switchCR(BKS_I);
				if (getBQ(cr)) switchCR(BQS_I);
			}
			if (toPiece == ROOK) {
				if (getWK(cr) && to == H1) switchCR(WKS_I);
				if (getWQ(cr) && to == A1) switchCR(WQS_I);
			}
		}
    }

	switchTurn();
}

void Pos::undoMove() {
	switchTurn();

	if (turn == BLACK) m_clock--;

	Move m = move_log.back();

	Color notturn = getOppositeColor(turn);
	
	Square from = getFrom(m);
	Square to = getTo(m);
	Piece fromPiece = getPieceAt(turn, to);
	Piece toPiece = to_piece_log.back();

	if (!isPromotion(m)) {
		removePiece(turn, to, fromPiece);
		setPiece(turn, from, fromPiece);
	}
	else {
		removePiece(turn, to, getPromotionType(m));
		setPiece(turn, from, PAWN);
	}

	if (getFlags(m)) {
		if (isCapture(m)) {
			if (!isEp(m)) setPiece(notturn, to, toPiece);
			else setPiece(notturn, to + (turn == WHITE ? -8 : 8), PAWN);
		}
        else if (isKingCastle(m)) {
            if (turn == WHITE) {
                removePiece(WHITE, F1, ROOK);
                setPiece(WHITE, H1, ROOK);
            }
            else {
                removePiece(BLACK, F8, ROOK);
                setPiece(BLACK, H8, ROOK);
            }
        }
        else if (isQueenCastle(m)) {
            if (turn == WHITE) {
                removePiece(WHITE, D1, ROOK);
                setPiece(WHITE, A1, ROOK);
            }
            else {
                removePiece(BLACK, D8, ROOK);
                setPiece(BLACK, A8, ROOK);
            }
        }
	}
	
	hashkey = hashkey_log.back();
	ep = ep_log.back();
	cr = cr_log.back();
	hm_clock = hm_clock_log.back();
	repetitions_index = repetitions_index_log.back();

	move_log.pop_back();
	to_piece_log.pop_back();
	hashkey_log.pop_back();
	ep_log.pop_back();
	cr_log.pop_back();
	hm_clock_log.pop_back();
	repetitions_index_log.pop_back();

}

void print(Pos& p, bool meta) {
	const static char white_piece_notation[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};
	const static char black_piece_notation[6] = {'p', 'n', 'b', 'r', 'q', 'k'};

	for (int r = 7; r >= 0; r--) {
		cout<<" +---+---+---+---+---+---+---+---+ "<<endl;
		for (int c = 0; c < 8; c++) {
			cout<<" | ";
			if (p.getPieceAt(WHITE, rc(r, c)) != PIECENONE) {
				cout<<white_piece_notation[p.getPieceAt(WHITE, rc(r, c))];
			}
			else if (p.getPieceAt(BLACK, rc(r, c)) != PIECENONE) {
				cout<<black_piece_notation[p.getPieceAt(BLACK, rc(r, c))];
			}
			else {
				cout<<' ';
			}
		}
		cout<<" | "<<endl;
	}
	cout<<" +---+---+---+---+---+---+---+---+ "<<endl;

	if (meta) {
		cout<<"turn: "<<(p.turn == WHITE ? "WHITE" : "BLACK")<<endl;
		cout<<"hashkey: "<< hex << uppercase << p.hashkey << nouppercase << dec << endl;
		cout<<"castle rights: ";
		if (getWK(p.cr)) cout<<"K";
		if (getWQ(p.cr)) cout<<"Q";
		if (getBK(p.cr)) cout<<"k";
		if (getBQ(p.cr)) cout<<"q";
		cout<<endl;
		cout<<"ep: "<<sqToNot(p.ep)<<endl;
		cout<<"halfmove clock: "<<to_string(p.hm_clock)<<endl;
		cout<<"move clock: "<<to_string(p.m_clock)<<endl;
		cout<<"fen: "<<getFen(p)<<endl;
	}
}

string getFen(Pos& p) {
	const static char white_piece_notation[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};
	const static char black_piece_notation[6] = {'p', 'n', 'b', 'r', 'q', 'k'};

	string fen = "";

	for (int r = 7; r >= 0; r--) {
		int space_count = 0;
		for (int c = 0; c < 8; c++) {
			if (p.getPieceAt(WHITE, rc(r, c)) != PIECENONE) {
				if (space_count != 0) fen += to_string(space_count);
				space_count = 0;
				fen += white_piece_notation[p.getPieceAt(WHITE, rc(r, c))];
			}
			else if (p.getPieceAt(BLACK, rc(r, c)) != PIECENONE) {
				if (space_count != 0) fen += to_string(space_count);
				space_count = 0;
				fen += black_piece_notation[p.getPieceAt(BLACK, rc(r, c))];
			}
			else space_count++;
		}
		if (space_count != 0) fen += to_string(space_count);
		if (r != 0) fen += '/';
	}

	fen += (p.turn == WHITE ? " w " : " b ");

	if (getWK(p.cr)) fen+="K";
	if (getWQ(p.cr)) fen+="Q";
	if (getBK(p.cr)) fen+="k";
	if (getBQ(p.cr)) fen+="q";

	fen += " " + sqToNot(p.ep);
	fen += " " + to_string(p.hm_clock);
	fen += " " + to_string(p.m_clock);

	return fen;
}

BB Pos::getAtkMask(Color c) {
	    BB mask = 0ULL;

    if (c == WHITE)
        mask |= ((getPieceMask(c, PAWN) & ~getColMask(7)) << 9) | ((getPieceMask(c, PAWN) & ~getColMask(0)) << 7);
    else
        mask |= ((getPieceMask(c, PAWN) & ~getColMask(0)) >> 9) | ((getPieceMask(c, PAWN) & ~getColMask(7)) >> 7);

    BB knights = getPieceMask(c, KNIGHT);
    while (knights) {
        int from = poplsb(knights);
        mask |= getKnightAtk(from);
    }

    BB nonking_occ = getOcc() & ~getPieceMask(getOppositeColor(c), KING);

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

    mask |= getKingAtk(lsb(getPieceMask(c, KING)));

    return mask;
}

bool Pos::isInCheck() {
	Square ksq = lsb(getPieceMask(turn, KING));
	BB rooks = getPieceMask(notturn, ROOK) | getPieceMask(notturn, QUEEN);
	BB bishops = getPieceMask(notturn, BISHOP) | getPieceMask(notturn, QUEEN);

	BB occ = getOcc();

	return  (getRookAtk(ksq, occ) & rooks) || 
			(getBishopAtk(ksq, occ) & bishops) || 
			(getKnightAtk(ksq) & getPieceMask(notturn, KNIGHT)) ||
			(getPawnAtk(turn, ksq) & getPieceMask(notturn, PAWN));
}

bool Pos::causesCheck(Move m) {
	makeMove(m);
	bool r = isInCheck();
	undoMove();
	return r;
}

bool Pos::threeRepetitions() {
	int count = 1;
	for (int i = hashkey_log.size()-4; i >= repetitions_index; i-=2) {
		if (hashkey_log[i] == hashkey) {
			count++;
			if (count == 3) return true;
		}
	}
	return false;
}

bool Pos::makeMove(string SAN) {
	vector<Move> moves = getLegalMoves(*this);
	Move move = MOVENONE;
	for (Move m : moves) {
		if (getSAN(m) == SAN) move = m;
	}
	if (move == MOVENONE) return false;
	makeMove(move);
	return true;
}

bool Pos::isGameOver() {
	if (threeRepetitions()) return true;
	if (hm_clock == 50) return true;
	if (!getLegalMoves(*this).size()) return true;
	if (!evalMat(*this, WHITE) && !evalMat(*this, BLACK)) return true;
	return false;
}

bool Pos::oneRepetition(int root) {
	for (int i = hashkey_log.size()-4; i >= max((int)repetitions_index, root); i-=2) if (hashkey_log[i] == hashkey) return true;
	return false;
}

bool Pos::insufficientMaterial() {
	if (getPieceMask(WHITE, PAWN) | getPieceMask(BLACK, PAWN) | 
		getPieceMask(WHITE, QUEEN) | getPieceMask(BLACK, QUEEN) |
		getPieceMask(WHITE, ROOK) | getPieceMask(BLACK, ROOK))
		return false;

	BB occ = getOcc();
	int piece_count = bitcount(occ);

	if (piece_count == 2) return true; //KvK
	if (bitcount(getPieceMask(WHITE, BISHOP) | getPieceMask(WHITE, KNIGHT)) > 1) return false;
	if (bitcount(getPieceMask(BLACK, BISHOP) | getPieceMask(BLACK, KNIGHT)) > 1) return false;
	if (piece_count == 3) return true;
	if (piece_count == 4) {
		if (getPieceMask(WHITE, BISHOP) && getPieceMask(BLACK, BISHOP) &&
		   ((lsb(getPieceMask(WHITE, BISHOP)) % 2) == (lsb(getPieceMask(BLACK, BISHOP)) % 2))) 
			return true;
	}

	return false;
}

void Pos::makeNullMove() {
	nullMovesMade++;
	switchTurn();
}
void Pos::undoNullMove() {
	nullMovesMade--;
	switchTurn();
}