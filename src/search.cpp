#include <iostream>

#include "attacks.h"
#include "move.h"
#include "movegen.h"
#include "pos.h"
#include "util.h"
#include "bits.h"
#include "timer.h"
#include "search.h"

template BB perft<true >(Pos& pos, Depth depth);
template BB perft<false>(Pos& pos, Depth depth);

template<bool DIVIDE>
BB perft(Pos& pos, Depth depth) {

	if (depth == 0) {
		if constexpr (DIVIDE) std::cout << "divide on depth zero, total: zero" << std::endl;
		return 1;
	}

	Timestamp start = 0;
	if constexpr (DIVIDE) start = get_current_ms();

	BB count = 0;
	std::vector<Move> moves = movegen::generate<movegen::LEGAL>(pos);

	if (depth == 1 && !DIVIDE) return moves.size();

	for (Move move : moves) {

		BB hashkey_before = pos.hashkey();

		if constexpr (DIVIDE) std::cout << move_to_string(move) << " ";
		
		pos.do_move(move);
		BB n = perft<false>(pos, depth - 1);
		
		if constexpr (DIVIDE) std::cout << std::to_string(n) << std::endl;
		
		pos.undo_move();
		
		assert(hashkey_before == pos.hashkey());

		count += n;
	}

	if constexpr (DIVIDE) std::cout << "total: " << std::to_string(count) << std::endl;
	if constexpr (DIVIDE) std::cout << "time: " << std::to_string(get_time_diff(start)) << " ms" << std::endl;
	if constexpr (DIVIDE) std::cout << "nps: " << std::to_string(count * 1000 / (get_time_diff(start) + 1)) << std::endl;

	return count;
}

/*


ThreadInfo::ThreadInfo(Pos& pos, std::string id_) {
	root_ply = pos.move_log.size();
	id = id_;
}

Eval search(Pos& pos, Depth depth, Eval alpha, Eval beta, ThreadInfo& ti, SearchInfo& si) {
	if (!ti.searching) return alpha;
	ti.nodes++;

	if (beta <= -MINMATE && beta != -INF) {
		beta--;
		if (alpha >= beta) return beta;
	}

	if (pos.insufficient_material()) return 0;
	if (pos.get_halfmove_clock() >= 4) {
		if (pos.get_halfmove_clock() >= 100) return 0;
		if (pos.one_repetition(ti.root_ply)) return 0;
		if (pos.three_repetitions()) return 0;
	}

	if (depth <= 0) return qsearch(pos, alpha, beta, ti, si);

	bool found = false;
	TTEntry* entry = si.tt.probe(pos.get_hashkey(), found);

	if (found && entry->get_depth() >= depth && entry->get_gen() == si.tt.gen) {
		if (entry->get_bound() == EXACT) return entry->get_eval();
		else if (entry->get_bound() == UB && entry->get_eval() < beta) beta = entry->get_eval();
		else if (entry->get_bound() == LB && entry->get_eval() > alpha) alpha = entry->get_eval();
		if (alpha >= beta) return beta;
	}

	std::vector<Move> moves = get_legal_moves(pos);

	if (moves.size() == 0) {
		Eval eval = pos.in_check() ? -INF : 0;
		entry->save(pos.get_hashkey(), eval, EXACT, DEPTH_MAX, MOVE_NONE, si.tt.gen);
		return eval;
	}
	
	int num_good;
	int num_boring;
	int num_bad;
	moves = order(moves, pos, ti, si, num_good, num_boring, num_bad, false);

	assert(moves.size());

	Eval besteval = -INF;
	Move bestmove = moves[0];

	for (int i = 0; i < moves.size(); i++) {
		Move& move = moves[i];

		pos.do_move(move);

		Eval eval;
		if (i < num_good) {
			eval = -search(pos, depth - 1, -beta, -max(alpha, besteval), ti, si);
		}
		else if (i < num_good + num_boring) {
			eval = -search(pos, depth - 2, -beta, -max(alpha, besteval), ti, si);
		}
		else {
			eval = -search(pos, depth - 3, -beta, -max(alpha, besteval), ti, si);
		}

		pos.undo_move();

		if (!ti.searching && found && i != 0) {
			break;
		}

		if (eval > MINMATE) eval--;

		if (eval > besteval) {
			besteval = eval;
			bestmove = move;
			
			if (besteval >= beta) {
				si.add_failhigh(pos, bestmove);
				break;
			}
		}
	}

	if (besteval <= alpha)		entry->save(pos.get_hashkey(), besteval, UB   , depth, bestmove, si.tt.gen);
	else if (besteval < beta)	entry->save(pos.get_hashkey(), besteval, EXACT, depth, bestmove, si.tt.gen);
	else						entry->save(pos.get_hashkey(), besteval, LB   , depth, bestmove, si.tt.gen);

	return besteval;
}

Eval qsearch(Pos& pos, Eval alpha, Eval beta, ThreadInfo& ti, SearchInfo& si) {
	ti.nodes++;
	Depth depth = pos.move_log.size() - ti.root_ply;
	if (depth > ti.seldepth) {
		ti.seldepth = depth;
	}

	if (beta <= -MINMATE && beta != -INF) {
		beta--;
		if (alpha >= beta) return beta;
	}

    Eval stand_pat = eval_pos(pos, alpha, beta);
    alpha = max(alpha, stand_pat);
    if (alpha >= beta) return beta;

	if (pos.insufficient_material()) return 0;
	// ONLY IF MOVES INCLUDE CHECKS
	if (pos.get_halfmove_clock() >= 4) {
		if (pos.get_halfmove_clock() >= 100) return 0;
		if (pos.one_repetition(ti.root_ply)) return 0;
		if (pos.three_repetitions()) return 0;
	}

	// king attacked evasion extension
	if (pos.in_check()) {
		return search(pos, 1, alpha, beta, ti, si);
	}

    std::vector<Move> moves = get_legal_moves(pos);

	if (moves.size() == 0) {
		if (pos.in_check()) return -INF;
		else return 0;
	}

	int num_good;
	int num_boring;
	int num_bad;
	moves = order(moves, pos, ti, si, num_good, num_boring, num_bad, true);

	assert(moves.size() == num_good);
    
    for (int i = 0; i < num_good; i++) {
		Move move = moves[i];

		pos.do_move(move);
		
		Eval eval = -qsearch(pos, -beta, -alpha, ti, si);
		
		pos.undo_move();
		
		if (eval > MINMATE) eval--;

		alpha = max(alpha, eval);

		if (alpha >= beta) {
			alpha = beta;
			break;
		}
    }
	
    return alpha;
}

mutex print_mutex;
void SearchInfo::launch(bool verbose) {
	stop();
	tis.clear();

    is_active = true;
    start_time = get_current_ms();
	last_depth_searched = 0;
    
    for (int i = 0; i < num_threads; i++) {
        tis.emplace_back(root_pos, "threadname_" + to_string(i));
    }

	std::vector<thread> threads;
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(&SearchInfo::worker, this, ref(tis[i]), ref(verbose));
    }

    while (true) {
        if (!is_active
            || (!ponder && get_time_diff(start_time) > max_time)) {
            break;
        }

        sleep_ms(10);
    }
    
    stop();
	print_uci_info();

    std::vector<Move> pv = tt.getPV(root_pos);
    std::vector<Move> moves = get_legal_moves(root_pos);
    print_mutex.lock();
    std::cout << "bestmove " + (pv.size() > 0 ? move_to_string(pv[0]) : (moves.size() ? move_to_string(moves[0]) : "(none)")) + (pv.size() > 1 ? " ponder " + move_to_string(pv[1]) : "") << std::endl;
    print_mutex.unlock();
    
    for (thread& thr : threads) {
        thr.join();
    }
}

void SearchInfo::stop() {
	for (ThreadInfo& ti : tis) {
        ti.searching = false;
    }
    is_active = false;
    last_depth_searched = 0;
}

void SearchInfo::clear() {
	for (int i = 0; i < 6; i++) {
		for (int sq = 0; sq < 64; sq++) {
			hist_hueristic[i][sq] = 0;
			cm_hueristic[i][sq] = MOVE_NONE;
		}
	}
	hist_score_max = 0;
	tt.clear();
}

void SearchInfo::worker(ThreadInfo& ti, bool verbose) {
	Pos root_copy = root_pos;

    while (ti.searching) {
        depth_increment_mutex.lock();
        Depth depth = ++last_depth_searched;
        depth_increment_mutex.unlock();

        if (depth > max_depth || depth < 0) break;
        
        search(root_copy, depth, -INF, INF, ti, *this);

		if (verbose && ti.searching) {
        	// std::cout << "thread " << ti.id << ":";
        	print_uci_info();
		}
    }
}

void SearchInfo::print_uci_info() {
    bool found = false;
    TTEntry* entry = tt.probe(root_pos.get_hashkey(), found);
    
    if (!found) return;

    BB nodes = get_nodes();
    Timestamp time_elapsed = get_time_diff(start_time);

    std::string msg = "";
    msg += "info";
    msg += " depth " + to_string(entry->get_depth());
	msg += " seldepth " + to_string(get_seldepth());
    msg += " score " + eval_to_string(entry->get_eval());
    msg += " nodes " + to_string(nodes);
    msg += " time " + to_string(time_elapsed);
    msg += " nps " + to_string(nodes * 1000 / (time_elapsed + 1));
    msg += " hashfull " + to_string(tt.hashfull());
    msg += " pv " + to_string(tt.getPV(root_pos)) + "\n";

    print_mutex.lock();
    std::cout << msg;
    print_mutex.unlock();
}

void timer(bool& target, Timestamp time) {
	Timestamp start = get_current_ms();
	while (target && get_time_diff(start) < time) {
		sleep_ms(10);
	}
	target = false;
}

Eval sea_gain(Pos& pos, Move move, Eval alpha) {
	pos.update_atks();

	Eval target_square = move::to_square(move);
	Eval target_piece_eval = get_piece_eval(pos.get_mailbox(pos.notturn, target_square));
	if (!(pos.get_atk(pos.notturn) & bb_of(target_square))) { // hanging
		return get_piece_eval(pos.get_mailbox(pos.notturn, target_square));
	}
	Eval result = -static_exchange_search(
		pos, 
		target_square, 
		pos.notturn, 
		-(target_piece_eval), 
		pos.get_occ() & ~bb_of(move::from_square(move)), 
		get_piece_eval(pos.get_mailbox(pos.turn, move::from_square(move))), 
		-INF, 
		-alpha);
	return result;
}

Eval static_exchange_search(Pos& pos, Square target_square, Color turn, Eval curr_mat, BB occ, Eval target_piece_eval, Eval alpha, Eval beta) {
	alpha = max(curr_mat, alpha);
	if (alpha >= beta) return beta;

	Square from = SQUARE_NONE;
	Piece from_piece = PIECE_NONE;
	for (Piece pt = PAWN; pt <= KING; pt++) {
		BB attackers = attacks::lookup(pt, target_square, opp(turn), occ)
			& pos.get_piece_mask(turn, pt)
			& occ;
		if (attackers) {
			from = lsb(attackers);
			from_piece = pt;
			break;
		}
	}

	if (from == SQUARE_NONE) return curr_mat;

	Eval result_after_move = -static_exchange_search(
		pos, 
		target_square, 
		opp(turn), 
		-(curr_mat + target_piece_eval), 
		occ & ~bb_of(from), 
		get_piece_eval(pos.get_mailbox(turn, from)), 
		-beta, 
		-alpha
		);
	return max(curr_mat, result_after_move);
}

Move get_best_move(Pos& pos, Depth depth) {
	SearchInfo si;
	ThreadInfo ti(pos);

	for (int d = 0; d < depth; d++) {
		search(pos, depth, -INF, INF, ti, si);
	}

	return si.tt.getPV(pos)[0];
}

*/