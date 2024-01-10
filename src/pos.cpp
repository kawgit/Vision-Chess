#include "pos.h"
#include "bits.h"
#include "types.h"
#include "move.h"
#include "util.h"
#include <iostream>
#include <map>
#include <string>
#include <cassert>
#include <cstring>

Pos::Pos(string fen) {

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
	Color target_turn = fen.find("w") != string::npos ? WHITE : BLACK;
	if (turn() != target_turn)
		switch_turn();

	// Castling rights
	if (piece_on(H1) == ROOK && color_on(H1) == WHITE && fen.find("K") != string::npos)
		switch_cr(H1);
	
	if (piece_on(A1) == ROOK && color_on(A1) == WHITE && fen.find("Q") != string::npos)
		switch_cr(A1);
	
	if (piece_on(H8) == ROOK && color_on(H8) == BLACK && fen.find("k") != string::npos)
		switch_cr(H8);
	
	if (piece_on(A8) == ROOK && color_on(A8) == BLACK && fen.find("q") != string::npos)
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
}

void print(Pos& pos, bool meta) {

	for (Rank rank = RANK_8; rank >= RANK_1; rank--) {

		cout << "+---+---+---+---+---+---+---+---+\n| ";

		for (File file = FILE_A; file <= FILE_H; file++) {

			Square square = square_of(rank, file);
			Spiece spiece = pos.spiece_on(square);

			cout << spiece_to_char(spiece) << " | ";

		}

		cout << endl;

	}

	cout << "+---+---+---+---+---+---+---+---+" << endl;

	if (meta) {
		cout << "turn: " << (pos.turn() == WHITE ? "WHITE" : "BLACK") << endl;
		cout << "hashkey: " <<  hex  <<  uppercase  <<  pos.hashkey()  <<  nouppercase  <<  dec  <<  endl;
		cout << "castle rights: ";
		if (bb_has(pos.cr(), H1)) cout << "K";
		if (bb_has(pos.cr(), A1)) cout << "Q";
		if (bb_has(pos.cr(), H8)) cout << "k";
		if (bb_has(pos.cr(), A8)) cout << "q";
		cout << endl;
		cout << "ep: " << square_to_string(pos.ep()) << endl;
		cout << "halfmove clock: " << to_string(pos.fifty_move_clock()) << endl;
		cout << "move clock: " << to_string(pos.move_clock()) << endl;
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
	assert(moving != PIECE_NONE);
	assert(color_on(from_square) == turn());
	assert(color_on(to_square) != turn());
	
	slice->move     = move;

	*(slice + 1) = *slice;
	slice++;

	rem_piece(turn(), moving, from_square);

	slice->fifty_move_clock = !(move::is_capture(move) || moving == PAWN) * (slice->fifty_move_clock + 1);

	move_clock_ += turn() == BLACK;

	if (move::is_capture(move)) {
		Square victim_square = move::capture_square(move);
		(slice - 1)->victim  = piece_on(victim_square);

		rem_piece(!turn(), slice->victim, victim_square);
	}

	add_piece(turn(), move::is_promotion(move) ? move::promotion_piece(move) : moving, to_square);

	bool new_ep = move::is_double_pawn_push(move) && ((shift<WEST>(bb_of(to_square)) | shift<EAST>(bb_of(to_square))) & pieces(!turn(), PAWN));
	set_ep(new_ep ? (to_square + from_square) / 2 : SQUARE_NONE);

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

	if (bb_of(from_square) & cr()) switch_cr(from_square);
	if (bb_of(to_square) & cr()) switch_cr(to_square);
	if (moving == KING) {
		BB our_castle_rooks = cr() & pieces(turn());
		while (our_castle_rooks) {
			Square castle_rook_square = poplsb(our_castle_rooks);
			switch_cr(castle_rook_square);
		}
	}
	
	switch_turn();

}

void Pos::undo_move() {
	assert(slice != slice_stack);

	Move move = (slice - 1)->move;
	
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
		add_piece(!turn(), (slice - 1)->victim, move::capture_square(move));
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
}

/*
void Pos::do_null_move() {
	move_log.push_back(MOVE_NULL);
	hashkey_log.push_back(hashkey);
	cr_log.push_back(cr);
	ep_log.push_back(ep);
	hm_clock_log.push_back(hm_clock);
	repetitions_index_log.push_back(repetitions_index);
	to_piece_log.push_back(PIECE_NONE);

	if (turn == BLACK) m_clock++;

	set_ep(SQUARE_NONE);

	switch_turn();
	null_moves_made++;
}

void Pos::undo_null_move() {
	if (turn == BLACK) m_clock--;

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

	switch_turn();

	null_moves_made--;
}
*/

/*
string get_fen(Pos& pos) {
	const static char white_piece_notation[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};
	const static char black_piece_notation[6] = {'p', 'n', 'b', 'r', 'q', 'k'};

	string fen = "";

	for (int r = 7; r >= 0; r--) {
		int space_count = 0;
		for (int c = 0; c < 8; c++) {
			if (pos.get_mailbox(WHITE, rc(r, c)) != PIECE_NONE) {
				if (space_count != 0) fen += to_string(space_count);
				space_count = 0;
				fen += white_piece_notation[pos.get_mailbox(WHITE, rc(r, c)) - PAWN];
			}
			else if (pos.get_mailbox(BLACK, rc(r, c)) != PIECE_NONE) {
				if (space_count != 0) fen += to_string(space_count);
				space_count = 0;
				fen += black_piece_notation[pos.get_mailbox(BLACK, rc(r, c)) - PAWN];
			}
			else space_count++;
		}
		if (space_count != 0) fen += to_string(space_count);
		if (r != 0) fen += '/';
	}

	fen += (pos.turn == WHITE ? " w " : " b ");

	if (getWK(pos.get_cr())) fen+="K";
	if (getWQ(pos.get_cr())) fen+="Q";
	if (getBK(pos.get_cr())) fen+="k";
	if (getBQ(pos.get_cr())) fen+="q";

	if (fen[fen.size() - 1] == ' ') fen+="-";

	fen += " " + square_to_string(pos.get_ep());
	fen += " " + to_string(pos.get_halfmove_clock());
	fen += " " + to_string(pos.get_move_clock());

	return fen;
}

int Pos::get_control_value(Color color, Square square) {
	int res = 0;
	BB occ = get_occ();
	res += bitcount(attacks::pawn(opp(color), square) & get_piece_mask(color, PAWN))
		 - bitcount(attacks::pawn(color, square) & get_piece_mask(opp(color), PAWN));
	res += bitcount(attacks::knight(square) & get_piece_mask(color, KNIGHT))
		 - bitcount(attacks::knight(square) & get_piece_mask(opp(color), KNIGHT));
	res += bitcount(attacks::bishop(square, occ) & (get_piece_mask(color, BISHOP) | get_piece_mask(color, QUEEN)))
		 - bitcount(attacks::bishop(square, occ) & (get_piece_mask(opp(color), BISHOP) | get_piece_mask(opp(color), QUEEN)));
	res += bitcount(attacks::rook(square, occ) & (get_piece_mask(color, ROOK) | get_piece_mask(color, QUEEN)))
		 - bitcount(attacks::rook(square, occ) & (get_piece_mask(opp(color), ROOK) | get_piece_mask(opp(color), QUEEN)));
	return res;
}

BB Pos::get_atk_mask(Color color) {
	BB mask = 0ULL;

    mask |= get_pawn_atk_mask(color);
    mask |= get_knight_atk_mask(color);
    mask |= get_bishop_atk_mask(color);
    mask |= get_rook_atk_mask(color);
    mask |= get_queen_atk_mask(color);
    mask |= get_king_atk_mask(color);

    return mask;
}

BB Pos::get_pawn_atk_mask(Color color) {
	BB mask = 0;
	if (color == WHITE) {
		mask |= shift<NORTH>(shift<WEST>(get_piece_mask(color, PAWN)) | shift<EAST>(get_piece_mask(color, PAWN)));
	}
	else {
		mask |= shift<SOUTH>(shift<WEST>(get_piece_mask(color, PAWN)) | shift<EAST>(get_piece_mask(color, PAWN)));
	}
	return mask;
}

BB Pos::get_knight_atk_mask(Color color) {
	BB mask = 0;
	BB knights = get_piece_mask(color, KNIGHT);
	while (knights) {
		int from = poplsb(knights);
		mask |= attacks::knight(from);
	}
	return mask;
}

BB Pos::get_bishop_atk_mask(Color color) {
	BB mask = 0;
	BB nonking_occ = get_occ() & ~get_piece_mask(opp(color), KING);
	BB bishops = get_piece_mask(color, BISHOP);
	while (bishops) {
		int from = poplsb(bishops);
		mask |= attacks::bishop(from, nonking_occ);
	}
	return mask;
}

BB Pos::get_rook_atk_mask(Color color) {
	BB mask = 0;
	BB nonking_occ = get_occ() & ~get_piece_mask(opp(color), KING);
	BB rooks = get_piece_mask(color, ROOK);
	while (rooks) {
		int from = poplsb(rooks);
		mask |= attacks::rook(from, nonking_occ);
	}
	return mask;
}

BB Pos::get_queen_atk_mask(Color color) {
	BB mask = 0;
	BB nonking_occ = get_occ() & ~get_piece_mask(opp(color), KING);
	BB queens = get_piece_mask(color, QUEEN);
	while (queens) {
		int from = poplsb(queens);
		mask |= attacks::queen(from, nonking_occ);
	}
	return mask;
}

BB Pos::get_king_atk_mask(Color color) {
	BB mask = 0;
	mask |= attacks::king(lsb(get_piece_mask(color, KING)));
	return mask;
}

bool Pos::in_check() {
	if (slice->has_updated_pins_and_checks) return get_num_checks() != 0;

	Square ksq = lsb(get_piece_mask(turn, KING));
	for (Piece pt = PAWN; pt <= QUEEN; pt++) {
		if (get_piece_mask(notturn, pt) && 
			(get_piece_mask(notturn, pt) & attacks::lookup(pt, ksq, turn, get_occ()))) {
			return true;
		}
	}
	return false;
}

bool Pos::three_repetitions() {
	int count = 1;
	for (int i = pi_log.size() - 5; i >= get_repetitions_index(); i -= 2) {
		if (pi_log[i].hashkey == get_hashkey()) {
			count++;
			if (count == 3) return true;
		}
	}
	return false;
}

bool Pos::do_move(string SAN) {
	vector<Move> moves = get_legal_moves(*this);
	Move move = MOVE_NONE;
	for (Move m : moves) {
		if (move_to_string(m) == SAN) move = m;
	}
	if (move == MOVE_NONE) return false;
	do_move(move);
	return true;
}

bool Pos::is_draw() {
	return three_repetitions()
		|| get_halfmove_clock() >= 100
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

void Pos::save(string path) {
	ofstream fs(path);
	if (!fs) {
		cout << "could not open file: " << path << endl;
		return;
	}

	vector<Move> move_log_copy = move_log;

	while (move_log.size()) {
		undo_move();
	}

	fs << "[FEN \"" + get_fen(*this) + "\"]\n";
	fs << "\n";

	for (int i = 0; i < move_log_copy.size(); i++) {
		if (i != 0) {
			if (in_check()) {
				fs << "+";
			}
			fs << " ";
		}
		do_move(move_log_copy[i]);
		if (i % 2 == 0) {
			fs << to_string(i / 2 + 1) + ".";
		}
		fs << move_to_string(move_log_copy[i]);
	}

	if (is_mate()) {
		fs << "#";
	}

	fs.close();
}

void Pos::update_pins_and_checks() {
	//pins and checks
	if (slice->has_updated_pins_and_checks) return;
	slice->has_updated_pins_and_checks = true;

	slice->num_checks = 0;
	for (int i = 0; i < 64; i++) slice->moveable_squares[i] = 0;
	slice->check_blocking_squares = ~0ULL;
	slice->pinned = 0;

	int ksq = lsb(get_piece_mask(turn, KING));
	
	bool is_pawn_check = false;
	if (attacks::pawn(turn, ksq) & get_piece_mask(notturn, PAWN)) {
        slice->num_checks++;
        slice->check_blocking_squares &= attacks::pawn(turn, ksq) & get_piece_mask(notturn, PAWN);
		is_pawn_check = true;
	}
    else if (attacks::knight(ksq) & get_piece_mask(notturn, KNIGHT)) {
        slice->num_checks++;
        slice->check_blocking_squares &= attacks::knight(ksq) & get_piece_mask(notturn, KNIGHT);
    }

    BB bishop_sliders = get_piece_mask(notturn, BISHOP) | get_piece_mask(notturn, QUEEN);
    BB total_bishop_rays = attacks::bishop(ksq, get_occ());
    BB bishop_checkers = bishop_sliders & total_bishop_rays;
    BB notturn_bishop_rays = attacks::bishop(ksq, get_occ(notturn));
    BB bishop_pinners = bishop_sliders & notturn_bishop_rays & ~bishop_checkers;
    if (bishop_checkers) {
        for (int d = NORTHEAST; d <= NORTHWEST; d += 2) {
            BB& ray = rays[ksq][d];
            if (ray & bishop_checkers) {
                add_check(ray & total_bishop_rays);
                //break; //ASSUMPTION: THERE CAN BE NO DOUBLE CHECK WITH 2 BISHOP SLIDERS
            }
        }
    }

	if (bishop_pinners) {
        for (int d = NORTHEAST; d <= NORTHWEST && bishop_pinners; d += 2) {
            BB& ray = rays[ksq][d];
            BB ally_mask = ray & notturn_bishop_rays & get_occ(turn);
            if (ray & bishop_pinners) {
                bishop_pinners &= ~ray;
                if (bitcount(ally_mask) == 1) {
                    add_pin(lsb(ally_mask), notturn_bishop_rays & ray);
				}
            }
        }
    }

	BB rook_sliders = get_piece_mask(notturn, ROOK) | get_piece_mask(notturn, QUEEN);
    BB total_rook_rays = attacks::rook(ksq, get_occ());
    BB rook_checkers = rook_sliders & total_rook_rays;
    BB notturn_rook_rays = attacks::rook(ksq, get_occ(notturn));
    BB rook_pinners = rook_sliders & notturn_rook_rays & ~rook_checkers;
    if (rook_checkers) {
        for (int d = NORTH; d <= WEST; d += 2) {
            BB& ray = rays[ksq][d];
            if (ray & rook_checkers) {
                add_check(ray & total_rook_rays);
                //break; //ASSUMPTION: THERE CAN BE NO DOUBLE CHECK WITH 2 ROOK SLIDERS
            }
        }
    }

    if (rook_pinners) {
        for (int d = NORTH; d <= WEST && rook_pinners; d += 2) {
            BB& ray = rays[ksq][d];
            BB ally_mask = ray & notturn_rook_rays & get_occ(turn);
            if (ray & rook_pinners) {
                rook_pinners &= ~ray;
                if (bitcount(ally_mask) == 1)
                    add_pin(lsb(ally_mask), notturn_rook_rays & ray);
            }
        }
    }

    //EN PASSANT CASE
    if (get_ep() != SQUARE_NONE && slice->num_checks != 2) {
        BB to_pawns = attacks::pawn(notturn, get_ep()) & get_piece_mask(turn, PAWN);
        while (to_pawns) {
            int from = poplsb(to_pawns);
            BB post = (get_occ() | bb_of(get_ep())) & (~bb_of(from)) & (~bb_of(get_ep() - (turn == WHITE ? 8 : -8)));
            if ((slice->num_checks == 1 && !is_pawn_check) || (attacks::bishop(ksq, post) & bishop_sliders) || (attacks::rook(ksq, post) & rook_sliders)) {
                if (slice->pinned & bb_of(from))
                    slice->moveable_squares[from] &= ~bb_of(get_ep());
                else {
                    slice->moveable_squares[from] = ~bb_of(get_ep());
                    slice->pinned |= bb_of(from);
                }
            }
            else {
                slice->moveable_squares[from] |= bb_of(get_ep());
            }
        }
    }
}


void Pos::update_atks() {
	if (slice->has_updated_atks) return;
	slice->has_updated_atks = true;
	get_atk(WHITE) = get_atk_mask(WHITE);
	get_atk(BLACK) = get_atk_mask(BLACK);
}

void Pos::update_sum_mat_squared() {
	if (slice->has_updated_sum_mat_squared) return;
	slice->has_updated_sum_mat_squared = true;
	get_sum_mat_squared(WHITE) = sum_mat_squared(*this, WHITE);
	get_sum_mat_squared(BLACK) = sum_mat_squared(*this, BLACK);
}
*/