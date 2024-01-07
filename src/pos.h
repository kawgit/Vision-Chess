#pragma once

#include <vector>
#include <stdint.h>
#include <string>
#include <cassert>
#include "bits.h"
#include "types.h"
#include "hash.h"
#include "move.h"
#include "util.h"
// #include "nnue.h"

struct Slice {
	BB 		hashkey;
	BB 		castle_rooks_bb;
	Clock 	fifty_move_clock;
	BB pinned_bb;
	Square 	ep;
	Move 	move;
	Piece 	victim;

	int num_checks;
	// BB check_blocking_squares = BB_FULL;
    // BB moveable_squares[N_SQUARES] = {};
};

class Pos {

	private:

	Color turn_ = BLACK;

	Clock move_clock_ = 1;
	
	BB color_bbs[N_COLORS]  = { BB_EMPTY };
	BB piece_bbs[N_PIECES]  = { BB_EMPTY };
	
	Color color_mailboxes[N_SQUARES] = { COLOR_NONE };
	Piece piece_mailboxes[N_SQUARES] = { PIECE_NONE };

	Slice  slice_stack[256];
	Slice* slice;

	public:

	// Pos accessors

	inline Color turn() const { return turn_; }

	inline Clock move_clock() const { return move_clock_; }

	inline BB pieces(const Color color, const Piece piece) const {
		assert(is_okay_color(color));
		assert(is_okay_piece(piece)); 
		return piece_bbs[piece] & color_bbs[color]; 
	}

	inline BB pieces(const Color color) const {
		assert(is_okay_color(color));
		return color_bbs[color]; 
	}

	inline BB pieces(const Piece piece) const {
		assert(is_okay_piece(piece)); 
		return piece_bbs[piece]; 
	}

	inline BB pieces() const {
		return color_bbs[WHITE] | color_bbs[BLACK]; 
	}

	inline Piece piece_on(const Square square) const {
		assert(is_okay_square(square));
		return piece_mailboxes[square];
	}

	inline Color color_on(const Square square) const {
		assert(is_okay_square(square));
		return color_mailboxes[square];
	}

	inline Spiece spiece_on(const Square square) const {
		assert(is_okay_square(square));
		return piece_on(square) != PIECE_NONE ? make_spiece(color_on(square), piece_on(square)) : SPIECE_NONE;
	}

	// Slice accessors

	inline BB hashkey() 			 const { return slice->hashkey; }
	inline BB cr() 			 		 const { return slice->castle_rooks_bb; }
	inline Square ep() 				 const { return slice->ep; }
	inline Clock fifty_move_clock()  const { return slice->fifty_move_clock; }

	// Modifiers
	
	inline void add_piece(const Color color, const Piece piece, const Square square) {
		
		assert(is_okay_color(color));
		assert(is_okay_piece(piece));
		assert(is_okay_square(square));

		assert(!(piece_bbs[piece] & bb_of(square)));
		assert(!(color_bbs[color] & bb_of(square)));
		assert(piece_mailboxes[square] == PIECE_NONE);
		assert(color_mailboxes[square] == COLOR_NONE);

		BB square_bb = bb_of(square);

		piece_bbs[piece] |= square_bb;
		color_bbs[color] |= square_bb;

		piece_mailboxes[square] = piece;
		color_mailboxes[square] = color;

		slice->hashkey ^= zobrist::psqt[color][piece][square];

	}

	inline void rem_piece(const Color color, const Piece piece, const Square square) {

		assert(is_okay_color(color));
		assert(is_okay_piece(piece));
		assert(is_okay_square(square));

		assert(piece_bbs[piece] & bb_of(square));
		assert(color_bbs[color] & bb_of(square));
		assert(piece_mailboxes[square] != PIECE_NONE);
		assert(color_mailboxes[square] != COLOR_NONE);

		BB square_bb = ~bb_of(square);

		piece_bbs[piece] &= square_bb;
		color_bbs[color] &= square_bb;

		piece_mailboxes[square] = PIECE_NONE;
		color_mailboxes[square] = COLOR_NONE;

		slice->hashkey ^= zobrist::psqt[color][piece][square];

	}

	inline void switch_turn() {
		turn_ = !turn_;
		slice->hashkey ^= zobrist::wtm;
	}

	inline void switch_cr(Square square) {
		slice->castle_rooks_bb ^= bb_of(square);
		slice->hashkey ^= zobrist::cr[square];
	}

	inline void set_ep(Square square) {
		slice->hashkey ^= zobrist::ep[slice->ep];
		slice->ep = square;
		slice->hashkey ^= zobrist::ep[slice->ep];
	}

	Pos(string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	void do_move(Move m);
	void undo_move();
	bool do_move(string SAN);
	void update_pins_and_checks();
	void update_atks();
	void update_sum_mat_squared();

	BB get_atk_mask(Color color);
	BB get_pawn_atk_mask(Color color);
	BB get_knight_atk_mask(Color color);
	BB get_bishop_atk_mask(Color color);
	BB get_rook_atk_mask(Color color);
	BB get_queen_atk_mask(Color color);
	BB get_king_atk_mask(Color color);

	bool in_check();
	bool three_repetitions();
	bool one_repetition(int root);
	bool insufficient_material();
	bool is_draw();
	bool is_mate();
	bool is_over();
	bool causes_check(Move move);

	// inline void add_check(BB block_squares) {
	// 	slice->check_blocking_squares &= block_squares;
	// 	slice->num_checks++;
	// }

	// inline void add_pin(int square, BB new_moveable_squares) {
	// 	slice->moveable_squares[square] = new_moveable_squares;
	// 	slice->pinned |= bb_of(square);
	// }
	
	// inline bool is_moveable(int from, int to) {
	// 	return ((!(get_pinned() & bb_of(from))) || (get_moveable_squares(from) & bb_of(to))) && (get_check_blocking_squares() & bb_of(to)); 
	// }
};

void print(Pos& pos, bool meta = false);

string get_fen(Pos& p);