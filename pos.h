#pragma once

#include <vector>
#include <stdint.h>
#include <string>
#include <cassert>
#include "bits.h"
#include "types.h"
#include "hash.h"

using namespace std;



class Pos {
	private:
	
	BB pieces_[2][6] = {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}};
	Piece mailboxes_[2][64] = { PIECE_NONE };
	
	public:

	const static int RESERVE_SIZE = 100;
	
	Color turn = WHITE;
	Color notturn = BLACK;
	BB hashkey = 0;
	CR cr = 0;
	Square ep = SQUARE_NONE;
	Clock hm_clock = 0;
	Clock m_clock = 1;
	Clock repetitions_index = 0;
	int null_moves_made = 0;

	vector<Move> move_log;
	vector<BB> hashkey_log;
	vector<CR> cr_log;
	vector<Piece> to_piece_log;
	vector<Square> ep_log;
	vector<Clock> hm_clock_log;
	vector<Clock> repetitions_index_log;
	

	Pos(string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

	vector<Move> generate();
	
	inline void switch_cr(CR_Index i) {
		hashkey ^= z_cr(i);
		cr ^= 1<<i;
	}

	inline void set_ep(Square s) {
		hashkey ^= z_ep(ep != SQUARE_NONE ? ep%8 : 8);
		ep = s;
		hashkey ^= z_ep(ep != SQUARE_NONE ? ep%8 : 8); 
	}

	inline void set_turn(Color c) { 
		if (c != turn) switch_turn(); 
	}

	inline void switch_turn() { 
		hashkey ^= z_turn();
		turn = notturn; 
		notturn = opp(turn); 
	}

	inline BB& pieces(Color c, Piece p) { 
		return pieces_[c == WHITE ? 1 : 0][p - PAWN];
	}

	inline Piece& mailboxes(Color c, Square s) { 
		assert(s < 64);
		return mailboxes_[c - BLACK][s]; 
	}
	
	inline Color color_at(Square s) { 
		assert(s < 64);
		return mailboxes(WHITE, s) != PIECE_NONE ? WHITE : 
			  (mailboxes(BLACK, s) != PIECE_NONE ? BLACK : COLOR_NONE); 
	}

	inline void setPiece(Color c, Square s, Piece p) { 
		assert(s < 64);
		hashkey ^= z_squares(c, p, s);
		pieces(c, p) |= get_BB(s);
		mailboxes(c, s) = p;
	}

	inline void removePiece(Color c, Square s, Piece p) {
		assert(s < 64);
		hashkey ^= z_squares(c, p, s);
		pieces(c, p) &= ~get_BB(s);
		mailboxes(c, s) = PIECE_NONE;
	}

	inline BB get_occ(Color c) { 
		return pieces(c, PAWN)
		 | pieces(c, KNIGHT)
		 | pieces(c, BISHOP)
		 | pieces(c, ROOK)
		 | pieces(c, KING)
		 | pieces(c, QUEEN); 
	}
	
	inline Move last_move() {
		return move_log.size() != 0 ? move_log.back() : MOVE_NONE;
	}

	inline Square last_from() {
		assert(last_move() != MOVE_NULL && last_move() != MOVE_NONE);
		return get_from(last_move());
	}

	inline Square last_to() {
		assert(last_move() != MOVE_NULL && last_move() != MOVE_NONE);
		return get_to(last_move());
	}

	inline Piece last_to_piece() {
		assert(last_move() != MOVE_NULL && last_move() != MOVE_NONE);
		return to_piece_log.back();
	}

	inline Piece last_from_piece() {
		assert(last_move() != MOVE_NULL && last_move() != MOVE_NONE);
		return mailboxes(turn, last_to());
	}

	inline BB get_occ() { 
		return get_occ(WHITE) | get_occ(BLACK);
	}
	
	void do_move(Move m);
	void undo_move();
	bool do_move(string SAN);

	void do_null_move();
	void undo_null_move();
	
	BB get_atk_mask(Color c);
	bool in_check();
	bool three_repetitions();
	bool one_repetition(int root);
	bool insufficient_material();
	bool is_draw();
	bool is_mate();
	bool is_over();

	bool causes_check(Move move);

	BB isolated_pawns(Color color);
	BB doubled_pawns(Color color);
	BB blocked_pawns(Color color);
	BB supported_pawns(Color color);
	BB passed_pawns(Color color);
	BB double_passed_pawns(Color color);

	void save(string path);
};



void print(Pos& p, bool meta = false);

string getFen(Pos& p);