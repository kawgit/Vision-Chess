#pragma once

#include <vector>
#include <stdint.h>
#include <string>
#include <cassert>
#include "bits.h"
#include "types.h"
#include "hash.h"
#include "move.h"
// #include "nnue.h"

struct Slice {
	BB 		hashkey = 0;
	BB 		castle_rooks_bb = 0;
	Clock 	fifty_move_clock = 0;
	Clock 	repetitions_index = 0;
	BB pinned_bb = 0;
	Square 	ep = FILE_NONE;
	Move 	move = 0;
	Piece 	captured = 0;

	int num_checks = 0;
	// BB check_blocking_squares = BB_FULL;
    // BB moveable_squares[N_SQUARES] = {};
};

class Pos {
	private:

	Color turn;
	Color notturn;

	Clock move_clock = 1;
	
	BB occ[2] 			   = { BB_EMPTY };
	BB piece_masks[2][6]   = { BB_EMPTY };
	Piece mailboxes[2][64] = { PIECE_NONE };

	Slice  slice_stack[256];
	Slice* slice;

	public:
	Pos(string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	void do_move(Move m);
	void undo_move();
	bool do_move(string SAN);
	void update_pins_and_checks();
	void update_atks();
	void update_sum_mat_squared();

	public:
	BB get_atk_mask(Color color);
	BB get_pawn_atk_mask(Color color);
	BB get_knight_atk_mask(Color color);
	BB get_bishop_atk_mask(Color color);
	BB get_rook_atk_mask(Color color);
	BB get_queen_atk_mask(Color color);
	BB get_king_atk_mask(Color color);

	public:
	bool in_check();
	bool three_repetitions();
	bool one_repetition(int root);
	bool insufficient_material();
	bool is_draw();
	bool is_mate();
	bool is_over();
	bool causes_check(Move move);

	int get_control_value(Color color, Square square);

	void save(string path);

	inline void add_piece(Color color, Piece piece, Square square) {
		
		assert(is_okay(color));
		assert(is_okay(piece));
		assert(is_okay(square));
		
		mailboxes[color][square] = piece;

		BB square_mask = get_BB(square);
		
		piece_masks[color][piece] |= square_mask;
		occ[color] 				  |= square_mask;
		
		slice->hashkey ^= z_squares(color, piece, square);
	}

	inline void rem_piece(Color color, Piece piece, Square square) {
		
		assert(is_okay(color));
		assert(is_okay(piece));
		assert(is_okay(square));
		
		mailboxes[color][square] = PIECE_NONE;
		
		BB square_mask = ~get_BB(square);

		piece_masks[color][piece] &= square_mask;
		occ[color] 				  &= square_mask;

		slice->hashkey ^= z_squares(color, piece, square);
	}

	// Pos accessors
	inline Color turn() { return turn; }
	inline Color notturn() { return notturn; }

	inline Clock move_clock() { return move_clock; }

	inline BB occ(Color color) { return occ[color]; }
	inline BB occ() { return occ[WHITE] | occ[BLACK]; }

	// Slice accessors
	inline BB hashkey() { return slice->hashkey; }
	inline Clock halfmove_clock() { return slice->fifty_move_clock; }
	inline Clock repetitions_index() { return slice->repetitions_index; }
	inline CR_Flag cr() { return slice->castle_rooks_bb; }
	inline Square ep() { return slice->ep; }



	inline Piece get_mailbox(Color color, Square square) {
		assert(square < 64);
		return mailboxes[color][square];
	}

	inline BB get_piece_mask(Color color, Piece piece) {
		return piece_masks[color][piece];
	}

	inline int get_num_checks() {
		assert(slice->has_updated_pins_and_checks);
		return slice->num_checks;
	}

	inline BB get_moveable_squares(Square square) {
		assert(slice->has_updated_pins_and_checks);
		return slice->moveable_squares[square];
	}

	inline BB get_check_blocking_squares() {
		assert(slice->has_updated_pins_and_checks);
		return slice->check_blocking_squares;
	}

	inline BB get_pinned() {
		assert(slice->has_updated_pins_and_checks);
		return slice->pinned;
	}

	inline BB get_atk(Color color) {
		assert(slice->has_updated_atks);
		return slice->atk[color];
	}
	
	inline Eval get_sum_mat_squared(Color color) {
		assert(slice->has_updated_sum_mat_squared);
		return slice->sum_mat_squared[color];
	}

	inline void add_check(BB block_squares) {
		slice->check_blocking_squares &= block_squares;
		slice->num_checks++;
	}

	inline void add_pin(int square, BB new_moveable_squares) {
		slice->moveable_squares[square] = new_moveable_squares;
		slice->pinned |= get_BB(square);
	}
	
	inline bool is_moveable(int from, int to) {
		return ((!(get_pinned() & get_BB(from))) || (get_moveable_squares(from) & get_BB(to))) && (get_check_blocking_squares() & get_BB(to)); 
	}
	
	inline void switch_cr(CR_Index i) {
		get_hashkey() ^= z_cr(i);
		get_cr() ^= 1<<i;
	}

	inline void set_ep(Square s) {
		get_hashkey() ^= z_ep(get_ep() != SQUARE_NONE ? get_ep() % 8 : 8);
		get_ep() = s;
		get_hashkey() ^= z_ep(get_ep() != SQUARE_NONE ? get_ep() % 8 : 8); 
	}

	inline void set_turn(Color c) { 
		if (c != turn) switch_turn(); 
	}

	inline void switch_turn() { 
		get_hashkey() ^= z_turn();
		turn = notturn; 
		notturn = opp(turn); 
	}
	
	inline Color color_at(Square s) { 
		assert(s < 64);
		return get_mailbox(WHITE, s) != PIECE_NONE ? WHITE : 
			  (get_mailbox(BLACK, s) != PIECE_NONE ? BLACK : COLOR_NONE); 
	}

	inline BB calc_occ(Color c) { 
		return get_piece_mask(c, PAWN)
		 | get_piece_mask(c, KNIGHT)
		 | get_piece_mask(c, BISHOP)
		 | get_piece_mask(c, ROOK)
		 | get_piece_mask(c, KING)
		 | get_piece_mask(c, QUEEN); 
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
		return slice->to_piece;
	}

	inline Piece last_from_piece() {
		assert(last_move() != MOVE_NULL && last_move() != MOVE_NONE);
		return get_mailbox(notturn, last_to());
	}
};

void print(Pos& p, bool meta = false);

string get_fen(Pos& p);