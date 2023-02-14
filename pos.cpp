#include "pos.h"
#include "bits.h"
#include "types.h"
#include "movegen.h"
#include "eval.h"
#include <iostream>
#include <map>
#include <string>
#include <cassert>
#include <fstream>

using namespace std;

static map<char, Piece> fenmap = {
	{'p', PAWN},
	{'n', KNIGHT},
	{'b', BISHOP},
	{'r', ROOK},
	{'q', QUEEN},
	{'k', KING},
};

Pos::Pos(string fen) {
	pi_log.emplace_back();

	for (int i = 0; i < 64; i++) {
		ref_mailbox(WHITE, i) = PIECE_NONE;
		ref_mailbox(BLACK, i) = PIECE_NONE;
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
			add_piece(color, rc(r, c), fenmap[(color == BLACK ? ch : (ch - 'A' + 'a'))]);
			c++;
		}
	}

	fen = fen.substr(i);

	set_turn(fen.find("w") != string::npos ? WHITE : BLACK);

	if (fen.find("K") != string::npos) switch_cr(WKS_I);
	if (fen.find("Q") != string::npos) switch_cr(WQS_I);
	if (fen.find("k") != string::npos) switch_cr(BKS_I);
	if (fen.find("q") != string::npos) switch_cr(BQS_I);

	for (int i = 0; i < 3; i++) fen = fen.substr(fen.find(" ")+1);

	if (fen[0] != '-') set_ep(string_to_square(fen));

	fen = fen.substr(fen.find(" ")+1);
	if (fen.size() == 0) goto end;

	ref_halfmove_clock() = stoi(fen.substr(0, fen.find(" ")));

	fen = fen.substr(fen.find(" ")+1);
	if (fen.size() == 0) goto end;

	ref_move_clock() = stoi(fen);

	end:

	pi_log.reserve(LOG_RESERVE_SIZE);
	move_log.reserve(LOG_RESERVE_SIZE);
	to_piece_log.reserve(LOG_RESERVE_SIZE);
}

void Pos::add_piece(Color c, Square s, Piece p) {
	assert(s < 64);
	ref_hashkey() ^= z_squares(c, p, s);
	ref_piece_mask(c, p) |= get_BB(s);
	ref_occ(c) |= get_BB(s);
	ref_occ() |= get_BB(s);
	ref_mailbox(c, s) = p;
	if (p != KING) ref_mat(c) += get_piece_eval(p);
}

void Pos::rem_piece(Color c, Square s, Piece p) {
	assert(s < 64);
	ref_hashkey() ^= z_squares(c, p, s);
	ref_piece_mask(c, p) &= ~get_BB(s);
	ref_occ(c) &= ~get_BB(s);
	ref_occ() = ref_occ(WHITE) | ref_occ(BLACK);
	ref_mailbox(c, s) = PIECE_NONE;
	if (p != KING) ref_mat(c) -= get_piece_eval(p);
}

void Pos::do_move(Move move) {
	assert(move != MOVE_NONE);

	// if (move == MOVE_NULL) {
	// 	do_null_move();
	// 	return;
	// }

	pi_log.emplace_back(pi_log.back());
	pi_log.back().has_updated_pins_and_checks = false;
	pi_log.back().has_updated_atks = false;
	pi_log.back().has_updated_sum_mat_squared = false;
	move_log.push_back(move);

	Square from = get_from(move);
	Square to = get_to(move);

	assert(to < 64);
	assert(from < 64);

	Piece from_piece = ref_mailbox(turn, from);
	Piece to_piece = ref_mailbox(notturn, to);
	to_piece_log.push_back(to_piece);
	
	if (to_piece == KING) {
		print(*this, true);
		cout << to_string(move_log) << endl;
	}

	assert(to_piece != KING);

	rem_piece(turn, from, from_piece);

	if (is_capture(move) || from_piece == PAWN) {
		ref_halfmove_clock() = 0;
		ref_repetitions_index() = pi_log.size();
	}
	else ref_halfmove_clock()++;
	if (turn == BLACK) ref_move_clock()++;

	if (!is_promotion(move)) add_piece(turn, to, from_piece);
	else add_piece(turn, to, get_promotion_type(move));

	set_ep(SQUARE_NONE);

	if (get_flags(move)) {
		if (is_capture(move)) {
			if (!is_ep(move)) rem_piece(notturn, to, to_piece);
			else rem_piece(notturn, to + (turn == WHITE ? -8 : 8), PAWN);
		}
		else if (is_double_pawn_push(move) && (get_rank_mask(to/8) & get_king_atk(to) & ref_piece_mask(notturn, PAWN))) {
			set_ep(to + (turn == WHITE ? -8 : 8));
		}
		else if (is_king_castle(move)) {
            if (turn == WHITE) {
                rem_piece(WHITE, H1, ROOK);
                add_piece(WHITE, F1, ROOK);
            }
            else {
                rem_piece(BLACK, H8, ROOK);
                add_piece(BLACK, F8, ROOK);
            }
		}
		else if (is_queen_castle(move)) {
            if (turn == WHITE) {
                rem_piece(WHITE, A1, ROOK);
                add_piece(WHITE, D1, ROOK);
            }
            else {
                rem_piece(BLACK, A8, ROOK);
                add_piece(BLACK, D8, ROOK);
            }
        }
	}

	if (ref_cr()) {
		if (turn == WHITE) {
			if (from_piece == ROOK) {
				if (getWK(ref_cr()) && from == H1) switch_cr(WKS_I);
				if (getWQ(ref_cr()) && from == A1) switch_cr(WQS_I);
			}
			if (from_piece == KING) {
				if (getWK(ref_cr())) switch_cr(WKS_I);
				if (getWQ(ref_cr())) switch_cr(WQS_I);
			}
			if (to_piece == ROOK) {
				if (getBK(ref_cr()) && to == H8) switch_cr(BKS_I);
				if (getBQ(ref_cr()) && to == A8) switch_cr(BQS_I);
			}
		}
		else {
			if (from_piece == ROOK) {
				if (getBK(ref_cr()) && from == H8) switch_cr(BKS_I);
				if (getBQ(ref_cr()) && from == A8) switch_cr(BQS_I);
			}
			if (from_piece == KING) {
				if (getBK(ref_cr())) switch_cr(BKS_I);
				if (getBQ(ref_cr())) switch_cr(BQS_I);
			}
			if (to_piece == ROOK) {
				if (getWK(ref_cr()) && to == H1) switch_cr(WKS_I);
				if (getWQ(ref_cr()) && to == A1) switch_cr(WQS_I);
			}
		}
    }

	switch_turn();
}

void Pos::undo_move() {
	Move move = move_log.back();
	
	assert(move != MOVE_NONE);

	// if (move == MOVE_NULL) {
	// 	undo_null_move();
	// 	return;
	// }

	switch_turn();

	if (turn == BLACK) ref_move_clock()--;
	
	Square from = get_from(move);
	Square to = get_to(move);
	Piece from_piece = ref_mailbox(turn, to);
	Piece to_piece = to_piece_log.back();

	if (!is_promotion(move)) {
		rem_piece(turn, to, from_piece);
		add_piece(turn, from, from_piece);
	}
	else {
		rem_piece(turn, to, get_promotion_type(move));
		add_piece(turn, from, PAWN);
	}

	if (get_flags(move)) {
		if (is_capture(move)) {
			if (!is_ep(move)) add_piece(notturn, to, to_piece);
			else add_piece(notturn, to + (turn == WHITE ? -8 : 8), PAWN);
		}
        else if (is_king_castle(move)) {
            if (turn == WHITE) {
                rem_piece(WHITE, F1, ROOK);
                add_piece(WHITE, H1, ROOK);
            }
            else {
                rem_piece(BLACK, F8, ROOK);
                add_piece(BLACK, H8, ROOK);
            }
        }
        else if (is_queen_castle(move)) {
            if (turn == WHITE) {
                rem_piece(WHITE, D1, ROOK);
                add_piece(WHITE, A1, ROOK);
            }
            else {
                rem_piece(BLACK, D8, ROOK);
                add_piece(BLACK, A8, ROOK);
            }
        }
	}

	pi_log.pop_back();
	move_log.pop_back();
	to_piece_log.pop_back();
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

void print(Pos& pos, bool meta) {
	const static char white_piece_notation[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};
	const static char black_piece_notation[6] = {'p', 'n', 'b', 'r', 'q', 'k'};

	for (int r = 7; r >= 0; r--) {
		cout << " +---+---+---+---+---+---+---+---+ " << endl;
		for (int c = 0; c < 8; c++) {
			cout << " | ";
			if (pos.ref_mailbox(WHITE, rc(r, c)) != PIECE_NONE) {
				cout << white_piece_notation[pos.ref_mailbox(WHITE, rc(r, c)) - PAWN];
			}
			else if (pos.ref_mailbox(BLACK, rc(r, c)) != PIECE_NONE) {
				cout << black_piece_notation[pos.ref_mailbox(BLACK, rc(r, c)) - PAWN];
			}
			else {
				cout << ' ';
			}
		}
		cout << " | " << endl;
	}
	cout << " +---+---+---+---+---+---+---+---+ " << endl;

	if (meta) {
		cout << "turn: " << (pos.turn == WHITE ? "WHITE" : "BLACK") << endl;
		cout << "hashkey: " <<  hex  <<  uppercase  <<  pos.ref_hashkey()  <<  nouppercase  <<  dec  <<  endl;
		cout << "castle rights: ";
		if (getWK(pos.ref_cr())) cout << "K";
		if (getWQ(pos.ref_cr())) cout << "Q";
		if (getBK(pos.ref_cr())) cout << "k";
		if (getBQ(pos.ref_cr())) cout << "q";
		cout << endl;
		cout << "ep: " << square_to_string(pos.ref_ep()) << endl;
		cout << "halfmove clock: " << to_string(pos.ref_halfmove_clock()) << endl;
		cout << "move clock: " << to_string(pos.ref_move_clock()) << endl;
		cout << "fen: " << get_fen(pos) << endl;
	}
}

string get_fen(Pos& pos) {
	const static char white_piece_notation[6] = {'P', 'N', 'B', 'R', 'Q', 'K'};
	const static char black_piece_notation[6] = {'p', 'n', 'b', 'r', 'q', 'k'};

	string fen = "";

	for (int r = 7; r >= 0; r--) {
		int space_count = 0;
		for (int c = 0; c < 8; c++) {
			if (pos.ref_mailbox(WHITE, rc(r, c)) != PIECE_NONE) {
				if (space_count != 0) fen += to_string(space_count);
				space_count = 0;
				fen += white_piece_notation[pos.ref_mailbox(WHITE, rc(r, c)) - PAWN];
			}
			else if (pos.ref_mailbox(BLACK, rc(r, c)) != PIECE_NONE) {
				if (space_count != 0) fen += to_string(space_count);
				space_count = 0;
				fen += black_piece_notation[pos.ref_mailbox(BLACK, rc(r, c)) - PAWN];
			}
			else space_count++;
		}
		if (space_count != 0) fen += to_string(space_count);
		if (r != 0) fen += '/';
	}

	fen += (pos.turn == WHITE ? " w " : " b ");

	if (getWK(pos.ref_cr())) fen+="K";
	if (getWQ(pos.ref_cr())) fen+="Q";
	if (getBK(pos.ref_cr())) fen+="k";
	if (getBQ(pos.ref_cr())) fen+="q";

	if (fen[fen.size() - 1] == ' ') fen+="-";

	fen += " " + square_to_string(pos.ref_ep());
	fen += " " + to_string(pos.ref_halfmove_clock());
	fen += " " + to_string(pos.ref_move_clock());

	return fen;
}

int Pos::get_control_value(Color color, Square square) {
	int res = 0;
	BB occ = ref_occ();
	res += bitcount(get_pawn_atk(opp(color), square) & ref_piece_mask(color, PAWN))
		 - bitcount(get_pawn_atk(color, square) & ref_piece_mask(opp(color), PAWN));
	res += bitcount(get_knight_atk(square) & ref_piece_mask(color, KNIGHT))
		 - bitcount(get_knight_atk(square) & ref_piece_mask(opp(color), KNIGHT));
	res += bitcount(get_bishop_atk(square, occ) & (ref_piece_mask(color, BISHOP) | ref_piece_mask(color, QUEEN)))
		 - bitcount(get_bishop_atk(square, occ) & (ref_piece_mask(opp(color), BISHOP) | ref_piece_mask(opp(color), QUEEN)));
	res += bitcount(get_rook_atk(square, occ) & (ref_piece_mask(color, ROOK) | ref_piece_mask(color, QUEEN)))
		 - bitcount(get_rook_atk(square, occ) & (ref_piece_mask(opp(color), ROOK) | ref_piece_mask(opp(color), QUEEN)));
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
		mask |= shift<NORTH>(shift<WEST>(ref_piece_mask(color, PAWN)) | shift<EAST>(ref_piece_mask(color, PAWN)));
	}
	else {
		mask |= shift<SOUTH>(shift<WEST>(ref_piece_mask(color, PAWN)) | shift<EAST>(ref_piece_mask(color, PAWN)));
	}
	return mask;
}

BB Pos::get_knight_atk_mask(Color color) {
	BB mask = 0;
	BB knights = ref_piece_mask(color, KNIGHT);
	while (knights) {
		int from = poplsb(knights);
		mask |= get_knight_atk(from);
	}
	return mask;
}

BB Pos::get_bishop_atk_mask(Color color) {
	BB mask = 0;
	BB nonking_occ = ref_occ() & ~ref_piece_mask(opp(color), KING);
	BB bishops = ref_piece_mask(color, BISHOP);
	while (bishops) {
		int from = poplsb(bishops);
		mask |= get_bishop_atk(from, nonking_occ);
	}
	return mask;
}

BB Pos::get_rook_atk_mask(Color color) {
	BB mask = 0;
	BB nonking_occ = ref_occ() & ~ref_piece_mask(opp(color), KING);
	BB rooks = ref_piece_mask(color, ROOK);
	while (rooks) {
		int from = poplsb(rooks);
		mask |= get_rook_atk(from, nonking_occ);
	}
	return mask;
}

BB Pos::get_queen_atk_mask(Color color) {
	BB mask = 0;
	BB nonking_occ = ref_occ() & ~ref_piece_mask(opp(color), KING);
	BB queens = ref_piece_mask(color, QUEEN);
	while (queens) {
		int from = poplsb(queens);
		mask |= get_queen_atk(from, nonking_occ);
	}
	return mask;
}

BB Pos::get_king_atk_mask(Color color) {
	BB mask = 0;
	mask |= get_king_atk(lsb(ref_piece_mask(color, KING)));
	return mask;
}

bool Pos::in_check() {
	if (pi_log.back().has_updated_pins_and_checks) return ref_num_checks() != 0;

	Square ksq = lsb(ref_piece_mask(turn, KING));
	for (Piece pt = PAWN; pt <= QUEEN; pt++) {
		if (ref_piece_mask(notturn, pt) && 
			(ref_piece_mask(notturn, pt) & get_piece_atk(pt, ksq, turn, ref_occ()))) {
			return true;
		}
	}
	return false;
}

bool Pos::three_repetitions() {
	int count = 1;
	for (int i = pi_log.size()-4; i >= ref_repetitions_index(); i-=2) {
		if (pi_log[i].hashkey == ref_hashkey()) {
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
		if (to_san(m) == SAN) move = m;
	}
	if (move == MOVE_NONE) return false;
	do_move(move);
	return true;
}

bool Pos::is_draw() {
	return three_repetitions()
		|| ref_halfmove_clock() >= 100
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
	for (int i = pi_log.size()-4; i >= max((int)ref_repetitions_index(), root); i-=2) {
		if (pi_log[i].hashkey == ref_hashkey()) {
			return true;
		}
	}
	return false;
}

bool Pos::insufficient_material() {
	if (ref_piece_mask(WHITE, PAWN) | ref_piece_mask(BLACK, PAWN) | 
		ref_piece_mask(WHITE, QUEEN) | ref_piece_mask(BLACK, QUEEN) |
		ref_piece_mask(WHITE, ROOK) | ref_piece_mask(BLACK, ROOK))
		return false;

	BB occ = ref_occ();
	int piece_count = bitcount(occ);

	if (piece_count == 2) return true; //KvK
	if (bitcount(ref_piece_mask(WHITE, BISHOP) | ref_piece_mask(WHITE, KNIGHT)) > 1) return false;
	if (bitcount(ref_piece_mask(BLACK, BISHOP) | ref_piece_mask(BLACK, KNIGHT)) > 1) return false;
	if (piece_count == 3) return true;
	if (piece_count == 4) {
		if (ref_piece_mask(WHITE, BISHOP) && ref_piece_mask(BLACK, BISHOP) &&
		   ((lsb(ref_piece_mask(WHITE, BISHOP)) % 2) == (lsb(ref_piece_mask(BLACK, BISHOP)) % 2))) 
			return true;
	}

	return false;
}

bool Pos::causes_check(Move move) {

	if (is_ep(move) || is_king_castle(move) || is_queen_castle(move) || is_promotion(move)) {
		do_move(move);
		bool result = in_check();
		undo_move();
		return result;
	}

	Square ksq = lsb(ref_piece_mask(notturn, KING));
	BB occ = ref_occ();
	BB rook_rays = get_rook_atk(ksq, occ);
	BB bishop_rays = get_bishop_atk(ksq, occ);
	BB from_mask = get_BB(get_from(move));
	BB to_mask = get_BB(get_to(move));
	
	if (rook_rays & from_mask
	 && get_rook_atk(ksq, (occ & ~from_mask) | to_mask) & (ref_piece_mask(turn, ROOK) | ref_piece_mask(turn, QUEEN))) {
		return true;
	}

	if (bishop_rays & from_mask
	 && get_bishop_atk(ksq, (occ & ~from_mask) | to_mask) & (ref_piece_mask(turn, BISHOP) | ref_piece_mask(turn, QUEEN))) {
		return true;
	}

	Piece piece = ref_mailbox(turn, get_from(move));

	switch (piece) {
		case PAWN:
			return get_pawn_atk(notturn, ksq) & to_mask;
			break;
		case KNIGHT:
			return get_knight_atk(ksq) & to_mask;
			break;
		case BISHOP:
			return get_bishop_atk(ksq, occ & ~from_mask) & to_mask;
			break;
		case ROOK:
			return get_rook_atk(ksq, occ & ~from_mask) & to_mask;
			break;
		case QUEEN:
			return get_queen_atk(ksq, occ & ~from_mask) & to_mask;
			break;
		case KING:
			return false;
			break;
	}
	
	return false;
}

BB Pos::isolated_pawns(Color color) {
	BB pawn_files = files_of(ref_piece_mask(color, PAWN));
	return ref_piece_mask(color, PAWN) & ~shift<EAST>(pawn_files) & ~shift<WEST>(pawn_files);
}

BB Pos::doubled_pawns(Color color) {
	BB pawns = ref_piece_mask(color, PAWN);
	return (pawns & (pawns >> 8)) | (pawns & (pawns >> 16)) | (pawns & (pawns >> 24));
}

BB Pos::blocked_pawns(Color color) {
	if (color == WHITE) return shift<NORTH>(ref_piece_mask(color, PAWN)) & ref_occ();
	else 				return shift<SOUTH>(ref_piece_mask(color, PAWN)) & ref_occ();
}

BB Pos::supported_pawns(Color color) {
	BB pawns = ref_piece_mask(color, PAWN);
	if (color == WHITE) return (pawns & shift<NORTH>(shift<EAST>(pawns))) | (pawns & shift<NORTH>(shift<WEST>(pawns)));
	else				return (pawns & shift<SOUTH>(shift<EAST>(pawns))) | (pawns & shift<SOUTH>(shift<WEST>(pawns)));
}

BB Pos::passed_pawns(Color color) {
	BB pawns = ref_piece_mask(color, PAWN);
	BB enemy_pawns = ref_piece_mask(opp(color), PAWN);
	BB passed = 0;
	while (pawns) {
		Square sq = poplsb(pawns);
		BB sq_bb = get_BB(sq);
		BB half = (sq_bb - 1);
		if (color == WHITE) half <<= 2;
		else half >>= 1;
		BB squares_ahead_of = files_of(sq_bb | shift<WEST>(sq_bb) | shift<EAST>(sq_bb)) & (color == WHITE ? ~half : half);
		if (!(squares_ahead_of & enemy_pawns)) passed |= sq_bb;
	}


	return passed;
}

BB Pos::double_passed_pawns(Color color) {
	return 0;
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
		fs << to_san(move_log_copy[i]);
	}

	if (is_mate()) {
		fs << "#";
	}

	fs.close();
}

void Pos::update_pins_and_checks() {
	//pins and checks
	if (pi_log.back().has_updated_pins_and_checks) return;
	pi_log.back().has_updated_pins_and_checks = true;

	pi_log.back().num_checks = 0;
	for (int i = 0; i < 64; i++) pi_log.back().moveable_squares[i] = 0;
	pi_log.back().check_blocking_squares = ~0ULL;
	pi_log.back().pinned = 0;

	int ksq = lsb(ref_piece_mask(turn, KING));
	
	bool is_pawn_check = false;
	if (get_pawn_atk(turn, ksq) & ref_piece_mask(notturn, PAWN)) {
        pi_log.back().num_checks++;
        pi_log.back().check_blocking_squares &= get_pawn_atk(turn, ksq) & ref_piece_mask(notturn, PAWN);
		is_pawn_check = true;
	}
    else if (get_knight_atk(ksq) & ref_piece_mask(notturn, KNIGHT)) {
        pi_log.back().num_checks++;
        pi_log.back().check_blocking_squares &= get_knight_atk(ksq) & ref_piece_mask(notturn, KNIGHT);
    }

    BB bishop_sliders = ref_piece_mask(notturn, BISHOP) | ref_piece_mask(notturn, QUEEN);
    BB total_bishop_rays = get_bishop_atk(ksq, ref_occ());
    BB bishop_checkers = bishop_sliders & total_bishop_rays;
    BB notturn_bishop_rays = get_bishop_atk(ksq, ref_occ(notturn));
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
            BB ally_mask = ray & notturn_bishop_rays & ref_occ(turn);
            if (ray & bishop_pinners) {
                bishop_pinners &= ~ray;
                if (bitcount(ally_mask) == 1) {
                    add_pin(lsb(ally_mask), notturn_bishop_rays & ray);
				}
            }
        }
    }

	BB rook_sliders = ref_piece_mask(notturn, ROOK) | ref_piece_mask(notturn, QUEEN);
    BB total_rook_rays = get_rook_atk(ksq, ref_occ());
    BB rook_checkers = rook_sliders & total_rook_rays;
    BB notturn_rook_rays = get_rook_atk(ksq, ref_occ(notturn));
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
            BB ally_mask = ray & notturn_rook_rays & ref_occ(turn);
            if (ray & rook_pinners) {
                rook_pinners &= ~ray;
                if (bitcount(ally_mask) == 1)
                    add_pin(lsb(ally_mask), notturn_rook_rays & ray);
            }
        }
    }

    //EN PASSANT CASE
    if (ref_ep() != SQUARE_NONE && pi_log.back().num_checks != 2) {
        BB to_pawns = get_pawn_atk(notturn, ref_ep()) & ref_piece_mask(turn, PAWN);
        while (to_pawns) {
            int from = poplsb(to_pawns);
            BB post = (ref_occ() | get_BB(ref_ep())) & (~get_BB(from)) & (~get_BB(ref_ep() - (turn == WHITE ? 8 : -8)));
            if ((pi_log.back().num_checks == 1 && !is_pawn_check) || (get_bishop_atk(ksq, post) & bishop_sliders) || (get_rook_atk(ksq, post) & rook_sliders)) {
                if (pi_log.back().pinned & get_BB(from))
                    pi_log.back().moveable_squares[from] &= ~get_BB(ref_ep());
                else {
                    pi_log.back().moveable_squares[from] = ~get_BB(ref_ep());
                    pi_log.back().pinned |= get_BB(from);
                }
            }
            else {
                pi_log.back().moveable_squares[from] |= get_BB(ref_ep());
            }
        }
    }
}

void Pos::update_atks() {
	if (pi_log.back().has_updated_atks) return;
	pi_log.back().has_updated_atks = true;
	ref_atk(WHITE) = get_atk_mask(WHITE);
	ref_atk(BLACK) = get_atk_mask(BLACK);
}

void Pos::update_sum_mat_squared() {
	if (pi_log.back().has_updated_sum_mat_squared) return;
	pi_log.back().has_updated_sum_mat_squared = true;
	ref_sum_mat_squared(WHITE) = sum_mat_squared(*this, WHITE);
	ref_sum_mat_squared(BLACK) = sum_mat_squared(*this, BLACK);
}