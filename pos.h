#pragma once

#include <vector>
#include <stdint.h>
#include <string>
#include <cassert>
#include "bits.h"
#include "types.h"
#include "hash.h"

using namespace std;

struct PosInfo {
	
	// need carry-over forward
	CR cr = 0;
	Square ep = EP_NONE;
	int num_checks = 0;
	Clock ply_clock = 0;
	Clock repetitions_index = 0;
	BB check_blocking_squares = ~0ULL;
	BB pinned = 0;
	BB hashkey = 0;
    BB moveable_squares[64] = {};
	BB atk[2] = {0, 0};
	bool has_updated_pins_and_checks = false;
	bool has_updated_atks = false;
};

class Pos {
	public:

	const static int MOVES_RESERVE_SIZE = 100;
	const static int LOG_RESERVE_SIZE = 1000;
	
	Color turn = WHITE;
	Color notturn = BLACK;
	
	private:
	// incrementally updated forwards and backwards
	int null_moves_made = 0;
	Clock move_clock = 1;
	Eval mat = 0;
	BB occ[3] = {0, 0, 0};
	Piece mailboxes[2][64] = { PIECE_NONE };
	BB piece_masks[2][6] = {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}};

	public:
	// recalculated forward, uses log to go backwards. 
	// in order to make code consice, all 
	// elements in this category are grouped into PosInfo
	vector<PosInfo> pi_log = {};
	vector<Move> move_log = {};
	vector<Piece> to_piece_log = {};

	public:
	Pos(string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	void do_move(Move m);
	void undo_move();
	bool do_move(string SAN);
	void update_pins_and_checks();
	void update_atks();

	// void do_null_move();
	// void undo_null_move();

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
	BB isolated_pawns(Color color);
	BB doubled_pawns(Color color);
	BB blocked_pawns(Color color);
	BB supported_pawns(Color color);
	BB passed_pawns(Color color);
	BB double_passed_pawns(Color color);

	void save(string path);

	void add_piece(Color c, Square s, Piece p);
	void rem_piece(Color c, Square s, Piece p);

	inline Square& ref_ep() {
		return pi_log.back().ep;
	}

	inline int& ref_num_checks() {
		return pi_log.back().num_checks;
	}

	inline CR& ref_cr() {
		return pi_log.back().cr;
	}

	inline Clock& ref_ply_clock() {
		return pi_log.back().ply_clock;
	}

	inline Clock& ref_repetitions_index() {
		return pi_log.back().repetitions_index;
	}

	inline BB& ref_moveable_squares(Square square) {
		return pi_log.back().moveable_squares[square];
	}

	inline BB& ref_check_blocking_squares() {
		return pi_log.back().check_blocking_squares;
	}

	inline BB& ref_pinned() {
		return pi_log.back().pinned;
	}

	inline BB& ref_hashkey() {
		return pi_log.back().hashkey;
	}

	inline Color& ref_turn() {
		return turn;
	}

	inline Color& ref_notturn() {
		return notturn;
	}

	inline int& ref_null_moves_made() {
		return null_moves_made;
	}

	inline Clock& ref_move_clock() {
		return move_clock;
	}

	inline Eval& ref_mat() {
		return mat;
	}

	inline BB& ref_atk(Color color) {
		if (!pi_log.back().has_updated_atks) update_atks();
		return pi_log.back().atk[color - BLACK];
	}

	inline BB& ref_occ(Color color = COLOR_NONE) {
		return occ[color];
	}

	inline Piece& ref_mailbox(Color color, Square square) {
		assert(square < 64);
		return mailboxes[color - BLACK][square];
	}

	inline BB& ref_piece_mask(Color color, Piece piece) {
		return piece_masks[color - BLACK][piece - PAWN];
	}

	inline void add_check(BB block_squares) {
		ref_check_blocking_squares() &= block_squares;
		ref_num_checks()++;
	}

	inline void add_pin(int square, BB new_moveable_squares) {
		ref_moveable_squares(square) = new_moveable_squares;
		ref_pinned() |= get_BB(square);
	}
	
	inline bool is_moveable(int from, int to) {
		return ((!(ref_pinned() & get_BB(from))) || (ref_moveable_squares(from) & get_BB(to))) && (ref_check_blocking_squares() & get_BB(to)); 
	}
	
	inline void switch_cr(CR_Index i) {
		ref_hashkey() ^= z_cr(i);
		ref_cr() ^= 1<<i;
	}

	inline void set_ep(Square s) {
		ref_hashkey() ^= z_ep(ref_ep() != SQUARE_NONE ? ref_ep() % 8 : 8);
		ref_ep() = s;
		ref_hashkey() ^= z_ep(ref_ep() != SQUARE_NONE ? ref_ep() % 8 : 8); 
	}

	inline void set_turn(Color c) { 
		if (c != turn) switch_turn(); 
	}

	inline void switch_turn() { 
		ref_hashkey() ^= z_turn();
		turn = notturn; 
		notturn = opp(turn); 
	}
	
	inline Color color_at(Square s) { 
		assert(s < 64);
		return ref_mailbox(WHITE, s) != PIECE_NONE ? WHITE : 
			  (ref_mailbox(BLACK, s) != PIECE_NONE ? BLACK : COLOR_NONE); 
	}

	inline BB calc_occ(Color c) { 
		return ref_piece_mask(c, PAWN)
		 | ref_piece_mask(c, KNIGHT)
		 | ref_piece_mask(c, BISHOP)
		 | ref_piece_mask(c, ROOK)
		 | ref_piece_mask(c, KING)
		 | ref_piece_mask(c, QUEEN); 
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
		return ref_mailbox(turn, last_to());
	}
};



void print(Pos& p, bool meta = false);

string get_fen(Pos& p);