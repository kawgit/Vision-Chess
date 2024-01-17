#include "pos.h"
#include "bits.h"
#include "types.h"
#include "move.h"
#include "util.h"
#include "movegen.h"
#include <iostream>
#include <map>
#include <string>
#include <cassert>
#include <cstring>

Pos::Pos(std::string fen) {

	std::memset(piece_bbs, BB_EMPTY, sizeof(piece_bbs));
	std::memset(color_bbs, BB_EMPTY, sizeof(color_bbs));
	std::memset(piece_mailboxes, PIECE_NONE, sizeof(piece_mailboxes));
	std::memset(color_mailboxes, COLOR_NONE, sizeof(color_mailboxes));

	slice = slice_stack;

	// Board
	Rank rank = RANK_8;
	File file = FILE_A;
	size_t fen_idx = 0;
	
	while (rank != RANK_1 || file != FILE_H + 1) {

		char c = fen[fen_idx++];
		
		if (c == '/') {
		
			file = FILE_A;
			rank--;
		
		}
		else if (c >= '0' && c <= '9') {
		
			file += c - '0';
		
		}
		else {
			
			Spiece spiece = char_to_spiece(c);

			Color color = color_of(spiece);
			Piece piece = type_of(spiece);
			Square square = square_of(rank, file);
			
			add_piece(color, piece, square);

			file++;
		
		}

	}

	fen = fen.substr(fen_idx);

	// Turn
	Color target_turn = fen.find("w") != std::string::npos ? WHITE : BLACK;
	if (turn() != target_turn)
		switch_turn();

	// Castling rights
	if (piece_on(H1) == ROOK && color_on(H1) == WHITE && fen.find("K") != std::string::npos)
		switch_cr(H1);
	
	if (piece_on(A1) == ROOK && color_on(A1) == WHITE && fen.find("Q") != std::string::npos)
		switch_cr(A1);
	
	if (piece_on(H8) == ROOK && color_on(H8) == BLACK && fen.find("k") != std::string::npos)
		switch_cr(H8);
	
	if (piece_on(A8) == ROOK && color_on(A8) == BLACK && fen.find("q") != std::string::npos)
		switch_cr(A8);
	
	for (size_t i = 0; i < 3; i++)
		fen = fen.substr(fen.find(" ") + 1);

	// En passant
	if (fen[0] != '-')
		set_ep(string_to_square(fen));
	fen = fen.substr(fen.find(" ") + 1);
	if (fen.size() == 0)
		return;

	// Fifty move rule
	slice->fifty_move_clock = stoi(fen.substr(0, fen.find(" ")));
	fen = fen.substr(fen.find(" ") + 1);
	if (fen.size() == 0)
		return;
	
	// Move clock
	move_clock_ = stoi(fen);

	assert_okay_pos(*this);
}

void print(const Pos& pos, bool meta) {

	for (Rank rank = RANK_8; rank >= RANK_1; rank--) {

		std::cout << "+---+---+---+---+---+---+---+---+\n| ";

		for (File file = FILE_A; file <= FILE_H; file++) {

			Square square = square_of(rank, file);
			Spiece spiece = pos.spiece_on(square);

			std::cout << spiece_to_char(spiece) << " | ";

		}

		std::cout << std::endl;

	}

	std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;

	if (meta) {
		std::cout << "turn: " << (pos.turn() == WHITE ? "WHITE" : "BLACK") << std::endl;
		std::cout << "hashkey: " <<  std::hex  <<  std::uppercase  <<  pos.hashkey()  <<  std::nouppercase  <<  std::dec  <<  std::endl;
		std::cout << "castle rights: ";
		if (bb_has(pos.cr(), H1)) std::cout << "K";
		if (bb_has(pos.cr(), A1)) std::cout << "Q";
		if (bb_has(pos.cr(), H8)) std::cout << "k";
		if (bb_has(pos.cr(), A8)) std::cout << "q";
		std::cout << std::endl;
		std::cout << "ep: " << square_to_string(pos.ep()) << std::endl;
		std::cout << "halfmove clock: " << std::to_string(pos.fifty_move_clock()) << std::endl;
		std::cout << "move clock: " << std::to_string(pos.move_clock()) << std::endl;
	}

}

void Pos::do_move(Move move) {
	Square from_square = move::from_square(move);
	Square to_square = move::to_square(move);
	Piece moving = piece_on(from_square);

	assert(is_okay_square(from_square));
	assert(is_okay_square(to_square));
	assert(piece_on(to_square) != KING);
	assert(piece_on(to_square) == PIECE_NONE || move::is_capture(move));
	assert(is_okay_piece(piece_on(from_square)));
	assert(moving != PIECE_NONE);
	assert(color_on(from_square) == turn());
	assert(color_on(to_square) != turn());
	
	slice->move = move;

	Slice* prev = slice;
	slice++;
	
	slice->hashkey = prev->hashkey;
	slice->castle_rooks_bb = prev->castle_rooks_bb;
	slice->fifty_move_clock = !(move::is_capture(move) || moving == PAWN) * (prev->fifty_move_clock + 1);
	slice->ep = prev->ep;
	
	slice->attacked_bb = BB_EMPTY;
	slice->checkers_bb = BB_FULL;
	slice->pinned_bb   = BB_FULL;
	slice->moveable_bb = BB_EMPTY;

	rem_piece(turn(), moving, from_square);

	move_clock_ += turn() == BLACK;

	if (move::is_capture(move)) {
		Square victim_square = move::capture_square(move);
		prev->victim  = piece_on(victim_square);

		rem_piece(notturn(), prev->victim, victim_square);
	}

	add_piece(turn(), move::is_promotion(move) ? move::promotion_piece(move) : moving, to_square);

	bool new_ep = move::is_double_pawn_push(move) && (shift<SOUTH>(attacks::pawn(to_square, WHITE)) & pieces(notturn(), PAWN));
	set_ep(new_ep ? (to_square + from_square) / 2 : SQUARE_NONE);

	if (bb_of(from_square) & cr()) switch_cr(from_square);
	if (bb_of(to_square) & cr()) switch_cr(to_square);
	if (moving == KING) {
		BB our_castle_rooks = cr() & pieces(turn());
		while (our_castle_rooks) {
			Square castle_rook_square = poplsb(our_castle_rooks);
			switch_cr(castle_rook_square);
		}
	}

	if (move::is_king_castle(move)) {
		Square offset = (turn() == BLACK) * (N_FILES * 7);
		Square rook_from = H1 + offset;
		Square rook_to   = F1 + offset;

		rem_piece(turn(), ROOK, rook_from);
		add_piece(turn(), ROOK, rook_to);
	}
	else if (move::is_queen_castle(move)) {
		Square offset = (turn() == BLACK) * (N_FILES * 7);
		Square rook_from = A1 + offset;
		Square rook_to   = D1 + offset;

		rem_piece(turn(), ROOK, rook_from);
		add_piece(turn(), ROOK, rook_to);
	}

	switch_turn();

	assert_okay_pos(*this);
}

void Pos::undo_move() {
	assert(slice != slice_stack);

	Slice* prev = slice - 1;

	Move move = prev->move;
	
	assert(move != MOVE_NONE);

	switch_turn();

	move_clock_ -= turn() == BLACK;
	
	Square from_square = move::from_square(move);
	Square to_square = move::to_square(move);
	Piece moving = piece_on(to_square);

	bool is_promotion = move::is_promotion(move);
	rem_piece(turn(), is_promotion ? move::promotion_piece(move) : moving, to_square);
	add_piece(turn(), is_promotion ? PAWN 						 : moving, from_square);

	if (move::is_capture(move))
		add_piece(notturn(), prev->victim, move::capture_square(move));
	else if (move::is_king_castle(move)) {
		Square offset = (turn() == BLACK) * (N_FILES * 7);
		Square rook_from = H1 + offset;
		Square rook_to   = F1 + offset;

		rem_piece(turn(), ROOK, rook_to);
		add_piece(turn(), ROOK, rook_from);
	}
	else if (move::is_queen_castle(move)) {
		Square offset = (turn() == BLACK) * (N_FILES * 7);
		Square rook_from = A1 + offset;
		Square rook_to   = D1 + offset;

		rem_piece(turn(), ROOK, rook_to);
		add_piece(turn(), ROOK, rook_from);
	}

	slice--;

	assert_okay_pos(*this);
}

void Pos::update_legal_info() {

	const Square king_square = lsb(pieces(turn(), KING));
	
	/// attacked_bb
	
	BB attacked_bb = BB_EMPTY;

	// add pawns
	const BB pawns = pieces(notturn(), PAWN);
	const BB we_pawns = shift<WEST>(pawns) | shift<EAST>(pawns);

	attacked_bb |= turn() == WHITE ? shift<SOUTH>(we_pawns) : shift<NORTH>(we_pawns);

	// add knights

	const BB knights = pieces(notturn(), KNIGHT);
	const BB we_knights = shift<WEST>(knights) | shift<EAST>(knights);
	const BB ns_knights = shift<NORTH>(knights) | shift<SOUTH>(knights);
	
	attacked_bb |= shift<NORTH>(shift<NORTH>(we_knights));
	attacked_bb |= shift<SOUTH>(shift<SOUTH>(we_knights));
	attacked_bb |= shift<EAST>(shift<EAST>(ns_knights));
	attacked_bb |= shift<WEST>(shift<WEST>(ns_knights));

	// add sliders

	const BB occupied = pieces() & ~bb_of(king_square);

	for (const Piece piece : {BISHOP, ROOK, QUEEN}) {

		BB piece_squares = pieces(notturn(), piece);

		while (piece_squares) {

			const Square piece_square = poplsb(piece_squares);

			attacked_bb |= attacks::lookup(piece, piece_square, occupied);

		}

	}

	// add king

	attacked_bb |= attacks::king(lsb(pieces(notturn(), KING)));
	
	slice->attacked_bb = attacked_bb;

	/// checkers_bb, pinned_bb, moveable_bb
	
	BB checkers_bb = (attacks::pawn(king_square, turn()) & pieces(notturn(), PAWN))
			       | (attacks::knight(king_square)           & pieces(notturn(), KNIGHT));
	BB pinned_bb   = BB_EMPTY;
	BB moveable_bb = checkers_bb ? checkers_bb : BB_FULL;

	BB sliders = ((pieces(notturn(), BISHOP) | pieces(notturn(), QUEEN)) & attacks::bishop(king_square, pieces(notturn())))
			   | ((pieces(notturn(), ROOK  ) | pieces(notturn(), QUEEN)) & attacks::rook  (king_square, pieces(notturn())));

	while (sliders) {
		
		const Square slider = poplsb(sliders);
		BB blockers = bb_segment(king_square, slider) & occupied;

		const bool is_check = !blockers;                                  // 0 blockers
		const bool is_pin   = !is_check && !bb_has_multiple(blockers);    // 1 blocker

		checkers_bb |= bb_of(slider) * is_check;
		pinned_bb   |= blockers * is_pin;
		moveable_bb &= ((bb_segment(king_square, slider) | bb_of(slider)) * is_check) | (BB_FULL * !is_check);

	}

	slice->checkers_bb = checkers_bb;
	slice->pinned_bb   = pinned_bb;
	slice->moveable_bb = moveable_bb;
}

bool Pos::is_legal(const Move move) const {

	const Square from_square = move::from_square(move);
	const Square to_square = move::to_square(move);
	
    if (move::is_ep(move)) {

        if (checkers() & ~pieces(PAWN))
			return false;

        const Square victim_square = square_of(rank_of(from_square), file_of(to_square));
        BB post_occupied = pieces();

        post_occupied &= ~bb_of(from_square);
        post_occupied |= bb_of(to_square);
        post_occupied &= ~bb_of(victim_square);

        const Square king_square = lsb(pieces(turn(), KING));
        const bool revealed_check = (attacks::bishop(king_square, post_occupied) & pieces(notturn(), BISHOP))
                                  | (attacks::rook  (king_square, post_occupied) & pieces(notturn(), ROOK))
                                  | (attacks::queen (king_square, post_occupied) & pieces(notturn(), QUEEN));

        return !revealed_check;

	}

	const BB span = bb_ray(lsb(pieces(turn(), KING)), from_square);

	const bool is_pinned = bb_has(pinned(), from_square);
	const bool is_onspan = bb_has(    span,   to_square);

	return !is_pinned || is_onspan;
}

bool Pos::do_move(std::string move_string) {
	
	std::vector<Move> legal_moves = movegen::generate<movegen::LEGAL>(*this);
	
	for (Move legal_move : legal_moves) {
		if (move_to_string(legal_move) == move_string) {
			do_move(legal_move);
			return true;
		}
	}
	
	return false;
}

bool Pos::in_check() {
	assert(checkers() != BB_FULL); 
	return checkers();
}

bool Pos::three_repetitions() {
	size_t count = 1;
	for (Slice* curr = slice - 5; curr >= slice_stack; curr -= 2) {
		if (curr->hashkey == slice->hashkey) {
			count++;
			if (count == 3)
				return true;
		}
	}
	return false;
}

/*

bool Pos::is_draw() {
	return three_repetitions()
		|| fifty_move_clock() >= 100
		|| insufficient_material()
		|| (!get_legal_moves(*this).size() && !in_check());
}

bool Pos::is_mate() {
	return !get_legal_moves(*this).size() && in_check();
}


bool Pos::is_over() {
	return is_draw() || is_mate();
}

bool Pos::one_repetition(int root) {
	int lb = max((int)get_repetitions_index(), root);
	for (int i = pi_log.size() - 5; i >= lb; i -= 2) {
		if (pi_log[i].hashkey == get_hashkey()) {
			return true;
		}
	}
	return false;
}

bool Pos::insufficient_material() {
	if (get_piece_mask(WHITE, PAWN) | get_piece_mask(BLACK, PAWN) | 
		get_piece_mask(WHITE, QUEEN) | get_piece_mask(BLACK, QUEEN) |
		get_piece_mask(WHITE, ROOK) | get_piece_mask(BLACK, ROOK))
		return false;

	BB occ = get_occ();
	int piece_count = bitcount(occ);

	if (piece_count == 2) return true; //KvK
	if (bitcount(get_piece_mask(WHITE, BISHOP) | get_piece_mask(WHITE, KNIGHT)) > 1) return false;
	if (bitcount(get_piece_mask(BLACK, BISHOP) | get_piece_mask(BLACK, KNIGHT)) > 1) return false;
	if (piece_count == 3) return true;
	if (piece_count == 4) {
		if (get_piece_mask(WHITE, BISHOP) && get_piece_mask(BLACK, BISHOP) &&
		   ((lsb(get_piece_mask(WHITE, BISHOP)) % 2) == (lsb(get_piece_mask(BLACK, BISHOP)) % 2))) 
			return true;
	}

	return false;
}

bool Pos::causes_check(Move move) {

	if (move::is_ep(move) || move::is_king_castle(move) || move::is_queen_castle(move) || move::is_promotion(move)) {
		do_move(move);
		bool result = in_check();
		undo_move();
		return result;
	}

	Square ksq = lsb(get_piece_mask(notturn, KING));
	BB occ = get_occ();
	BB rook_rays = attacks::rook(ksq, occ);
	BB bishop_rays = attacks::bishop(ksq, occ);
	BB from_mask = bb_of(move::from_square(move));
	BB to_mask = bb_of(move::to_square(move));
	
	if (rook_rays & from_mask
	 && attacks::rook(ksq, (occ & ~from_mask) | to_mask) & (get_piece_mask(turn, ROOK) | get_piece_mask(turn, QUEEN))) {
		return true;
	}

	if (bishop_rays & from_mask
	 && attacks::bishop(ksq, (occ & ~from_mask) | to_mask) & (get_piece_mask(turn, BISHOP) | get_piece_mask(turn, QUEEN))) {
		return true;
	}

	Piece piece = get_mailbox(turn, move::from_square(move));

	switch (piece) {
		case PAWN:
			return attacks::pawn(notturn, ksq) & to_mask;
			break;
		case KNIGHT:
			return attacks::knight(ksq) & to_mask;
			break;
		case BISHOP:
			return attacks::bishop(ksq, occ & ~from_mask) & to_mask;
			break;
		case ROOK:
			return attacks::rook(ksq, occ & ~from_mask) & to_mask;
			break;
		case QUEEN:
			return attacks::queen(ksq, occ & ~from_mask) & to_mask;
			break;
		case KING:
			return false;
			break;
	}
	
	return false;
}
*/

void assert_okay_pos(const Pos& pos) {

	#ifndef NDEBUG

		BB hashkey = 0;

		for (Square square = A1; square <= H8; square++) {
			const Piece piece = pos.piece_on(square);
			const Color color = pos.color_on(square);

			assert((color == COLOR_NONE) == (piece == PIECE_NONE));

			if (piece != PIECE_NONE) {
				assert(is_okay_piece(piece));
				assert(is_okay_color(color));

				hashkey ^= zobrist::psqt[color][piece][square];
			}
		}

		assert(is_okay_color(pos.turn()));
		assert(is_okay_color(pos.notturn()));
		assert(pos.notturn() == !pos.turn());

		if (pos.turn() == WHITE)
			hashkey ^= zobrist::wtm;

		BB cr_bb = pos.cr();
		while (cr_bb) {
			Square cr = poplsb(cr_bb);
			assert(pos.piece_on(cr) == ROOK);
			hashkey ^= zobrist::cr[cr];
		}


		const Square forward_offset = pos.turn() == WHITE ? 8 : -8;

		if (pos.ep() != SQUARE_NONE) {

			assert(is_okay_square(pos.ep()));

			Square victim_square = pos.ep() - forward_offset;
			
			assert(is_okay_square(victim_square));
			assert(pos.piece_on(victim_square) == PAWN);
			assert(pos.color_on(victim_square) == pos.notturn());

			assert(attacks::pawn(pos.ep(), pos.notturn()) & pos.pieces(pos.turn(), PAWN));

			hashkey ^= zobrist::ep[pos.ep()];

		}

		assert(hashkey == pos.hashkey());

	#endif
}