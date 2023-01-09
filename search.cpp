#include "search.h"
#include "pos.h"
#include "bits.h"
#include "timer.h"
#include "tt.h"
#include "movegen.h"
#include "hash.h"
#include "eval.h"
#include "order.h"
#include "uci.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>

using namespace std;

#define REDUCTION(d, i, f) ((d)-1)


ThreadInfo::ThreadInfo(Pos& p, string id_) {
	root_ply = p.move_log.size();
	id = id_;
}

BB perft(Pos &p, Depth depth, bool divide) {
	if (depth == 0) return 1;

	Timestamp start;
	if (divide) start = get_current_ms();

	BB count = 0;
	vector<Move> moves = get_legal_moves(p);

	//if (depth == 1 && !divide) return moves.size();

	if (divide) cout << moves.size() << endl;

	for (Move move : moves) {
		p.do_move(move);
		BB n = perft(p, depth-1, false);
		p.undo_move();

		if (divide) cout << getSAN(move) << " " << to_string(n) << endl;
		count += n;
	}

	if (divide) cout << "total: " << to_string(count) << endl;
	if (divide) cout << "time: " << to_string(get_time_diff(start)) <<" ms" << endl;

	return count;
}

void perftTest() {
	vector<string> fens = {
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
		"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
		"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
		"r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
		"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
		"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ",
		};
	vector<BB>   counts = {
		4865609,
		193690690,
		674624,
		15833292,
		15833292,
		89941194,
		164075551,
	};

	for (int i = 0; i < fens.size(); i++) {
		Pos p(fens[i]);
		Timestamp start = get_current_ms();
		BB count = perft(p, 5, false);
		cout << (count == counts[i] ? "pass " : "fail ") 
			<< "time: " << std::setw (6) << to_string(get_time_diff(start)) << "ms " 
			<< "nps: " <<std::setw (8) << to_string(count*1000/get_time_diff(start)) 
			<< " fen: " << fens[i] 
			<< endl;
	}

}

Eval search(Pos& pos, Depth depth, Eval alpha, Eval beta, ThreadInfo& ti, SearchInfo& si) {
	if (!ti.searching) return 0;
	ti.nodes++;

	if (beta <= -MINMATE && beta != -INF) {
		beta--;
		if (alpha >= beta) return beta;
	}

	if (pos.insufficient_material()) return 0;
	if (pos.hm_clock >= 4) {
		if (pos.hm_clock == 50) return 0;
		if (pos.one_repetition(ti.root_ply)) return 0;
		if (pos.three_repetitions()) return 0;
	}

	if (depth <= 0) return qsearch(pos, alpha, beta, ti, si);

	bool found = false;
	TTEntry* entry = si.tt.probe(pos.hashkey, found);

	if (found && entry->get_depth() >= depth && entry->get_gen() == si.tt.gen) {
		if (entry->get_bound() == EXACT) return entry->get_eval();
		else if (entry->get_bound() == UB && entry->get_eval() < beta) beta = entry->get_eval();
		else if (entry->get_bound() == LB && entry->get_eval() > alpha) alpha = entry->get_eval();
		if (alpha >= beta) return beta;
	}

	vector<Move> moves = get_legal_moves(pos);

	if (moves.size() == 0) {
		Eval eval = pos.in_check() ? -INF : 0;
		entry->save(pos.hashkey, eval, EXACT, depth, MOVE_NONE, si.tt.gen);
		return eval;
	}
	
	int interesting = 0;
	moves = order(moves, pos, ti, si, interesting);

	Eval besteval = -INF;
	Move bestmove = moves[0];

	for (int i = 0; i < moves.size(); i++) {
		Move& move = moves[i];

		pos.do_move(move);

#ifndef AGGR_PRUNE
		Eval eval = -search(pos, depth - (i > interesting ? depth / 8 + 2 : 1), -beta, -max(alpha, besteval), ti, si);
#endif
#ifdef AGGR_PRUNE
		Eval eval = -search(pos, depth - (i > interesting ? depth / 4 + 2 : 1), -beta, -max(alpha, besteval), ti, si);
#endif

		pos.undo_move();

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

	if (!ti.searching) return 0;

	if (besteval <= alpha)		entry->save(pos.hashkey, besteval, UB   , depth, bestmove, si.tt.gen);
	else if (besteval < beta)	entry->save(pos.hashkey, besteval, EXACT, depth, bestmove, si.tt.gen);
	else						entry->save(pos.hashkey, besteval, LB   , depth, bestmove, si.tt.gen);

	return besteval;
}

Eval qsearch(Pos& pos, Eval alpha, Eval beta, ThreadInfo& ti, SearchInfo& si) {
	if (!ti.searching) return 0;
	ti.nodes++;

	if (beta <= -MINMATE && beta != -INF) {
		beta--;
		if (alpha >= beta) return beta;
	}

    Eval stand_pat = eval_pos(pos, alpha, beta);
    alpha = max(alpha, stand_pat);
    if (alpha >= beta) return beta;

	if (pos.insufficient_material()) return 0;
	// ONLY IF MOVES INCLUDE CHECKS
	if (pos.hm_clock >= 4) {
		if (pos.hm_clock == 50) return 0;
		if (pos.one_repetition(ti.root_ply)) return 0;
		if (pos.three_repetitions()) return 0;
	}

#ifndef NO_QSEARCH_EVASION
	if (pos.in_check()) {
		return search(pos, 1, alpha, beta, ti, si);
	}
#endif

    vector<Move> moves = get_legal_moves(pos);

	if (moves.size() == 0) {
		if (pos.in_check()) return -INF;
		else return 0;
	}

	int interesting = moves.size();
	moves = order(moves, pos, ti, si, interesting, true);
    
    for (int i = 0; i < interesting; i++) {
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
	clear();

    is_active = true;
    start_time = get_current_ms();
	last_depth_searched = 0;
    
    for (int i = 0; i < num_threads; i++) {
        tis.emplace_back(root_pos, "threadname_" + to_string(i));
    }

	vector<thread> threads;
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(&SearchInfo::worker, this, ref(tis[i]), ref(verbose));
    }

    while (true) {
        if (!is_active
            || (!ponder && get_time_diff(start_time) > max_time)) {
            break;
        }

        sleep(10);
    }
    
    stop();
	print();

    vector<Move> pv = tt.getPV(root_pos);
    vector<Move> moves = get_legal_moves(root_pos);
    print_mutex.lock();
    cout << "bestmove " + (pv.size() > 0 ? getSAN(pv[0]) : (moves.size() ? getSAN(moves[0]) : "(none)")) + (pv.size() > 1 ? " ponder " + getSAN(pv[1]) : "") << endl;
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

        // if (depth > max_depth || depth < 0) break;
        
        search(root_copy, depth, -INF, INF, ti, *this);

		if (verbose && ti.searching) {
        	// cout << "thread " << ti.id << ":";
        	print();
		}
    }
}

void SearchInfo::print() {
    bool found = false;
    TTEntry* entry = tt.probe(root_pos.hashkey, found);
    
    if (!found) return;

    BB nodes = get_nodes();
    Timestamp time_elapsed = get_time_diff(start_time);

    string msg = "";
    msg += "info";
    msg += " depth " + to_string(entry->get_depth());
    msg += " score " + eval_to_string(entry->get_eval());
    msg += " nodes " + to_string(nodes);
    msg += " time " + to_string(time_elapsed);
    msg += " nps " + to_string(nodes * 1000 / (time_elapsed + 1));
    msg += " hashfull " + to_string(tt.hashfull());
    msg += " pv " + to_string(tt.getPV(root_pos)) + "\n";

    print_mutex.lock();
    cout << msg;
    print_mutex.unlock();
}

void timer(bool& target, Timestamp time) {
	Timestamp start = get_current_ms();
	while (target && get_time_diff(start) < time) {
		sleep(10);
	}
	target = false;
}