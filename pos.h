#pragma once

#include <vector>
#include <stdint.h>
#include <string>
#include "bits.h"
#include "types.h"
#include "hash.h"
#include "nnue.h"

using namespace std;



class Pos {
	public:

	const static int RESERVE_SIZE = 100;

	Color turn = WHITE;
	Color notturn = BLACK;
	BB pieces[2][6] = {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}};
	Piece mailboxes[2][64] = { PIECENONE };
	BB hashkey = 0;
	CR cr = 0;
	Square ep = SQUARENONE;
	Clock hm_clock = 0;
	Clock m_clock = 1;
	Clock repetitions_index = 0;
	int nullMovesMade = 0;
	NNUE* nnue = nullptr;

	vector<Move> move_log;
	vector<BB> hashkey_log;
	vector<CR> cr_log;
	vector<Piece> to_piece_log;
	vector<Square> ep_log;
	vector<Clock> hm_clock_log;
	vector<Clock> repetitions_index_log;
	

	Pos(string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	inline void switchCR(CR_Index i) {
		hashkey ^= z_cr[i];
		cr ^= 1<<i;
	}
	inline void setEp(Square s) {
		hashkey ^= z_ep[ep != SQUARENONE ? ep%8 : 8];
		hashkey ^= z_ep[s != SQUARENONE ? s%8 : 8]; 
		ep = s;
	}
	inline void setTurn(Color c) { if (c != turn) switchTurn(); }
	inline void switchTurn() { 
		hashkey ^= z_turn;
		turn = notturn; 
		notturn = getOppositeColor(notturn); 
	}
	inline BB& getPieceMask(Color c, Piece p) { return pieces[c == WHITE ? 1 : 0][p]; }
	inline Square getPieceAt(Color c, Square s) { return mailboxes[c][s]; }
	inline void setPiece(Color c, Square s, Piece p) { 
		hashkey ^= z_squares[c == WHITE ? 1 : 0][p][s];
		getPieceMask(c, p) |= getBB(s);
		mailboxes[c][s] = p;

		if (nnue != nullptr) 
			nnue->set_piece(lsb(getPieceMask(turn, KING)), lsb(getPieceMask(notturn, KING)), c, s, p);
	}
	inline void removePiece(Color c, Square s, Piece p) {
		hashkey ^= z_squares[c == WHITE ? 1 : 0][p][s];
		getPieceMask(c, p) &= ~getBB(s);
		mailboxes[c][s] = PIECENONE;

		if (nnue != nullptr) 
			nnue->rem_piece(lsb(getPieceMask(turn, KING)), lsb(getPieceMask(notturn, KING)), c, s, p);
	}

	BB getAtkMask(Color c);
	bool isInCheck();
	bool causesCheck(Move m);
	bool threeRepetitions();
	bool oneRepetition(int root);
	bool insufficientMaterial();
	bool isGameOver();
	int getResult();

	inline BB getOcc(Color c) { return getPieceMask(c, PAWN) | getPieceMask(c, KNIGHT) |getPieceMask(c, BISHOP) |getPieceMask(c, ROOK) |getPieceMask(c, KING) |getPieceMask(c, QUEEN); }
	inline BB getOcc() { return getOcc(WHITE) | getOcc(BLACK); }
	void makeMove(Move m);
	void undoMove();
	bool makeMove(string SAN);
	void makeNullMove();
	void undoNullMove();
};



void print(Pos& p, bool meta = false);

string getFen(Pos& p);