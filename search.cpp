#include "search.h"
#include "pos.h"
#include "bits.h"
#include "timer.h"
#include "tt.h"
#include "movegen.h"
#include "hash.h"
#include "eval.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>

using namespace std;

BB perft(Pos &p, Depth depth, bool divide) {
	if (depth == 0) return 1;

	Timestamp start;
	if (divide) start = get_current_ms();

	BB count = 0;
	vector<Move> moves = getLegalMoves(p);

	if (depth == 1 && !divide) return moves.size();

	if (divide) cout<<moves.size()<<endl;

	for (Move move : moves) {

		p.makeMove(move);
		BB n = perft(p, depth-1, false);
		p.undoMove();

		if (divide) cout<<getSAN(move)<<" "<<to_string(n)<<endl;
		count += n;
	}

	if (divide) cout<<"total: "<<to_string(count)<<endl;
	if (divide) cout<<"time: "<<to_string(get_time_diff(start))<<"ms"<<endl;

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
		cout<<(count == counts[i] ? "pass " : "fail ")<<"time: "<<std::setw (6)<<to_string(get_time_diff(start))<<"ms "<<"nps: "<<std::setw (8)<<to_string(count*1000/get_time_diff(start))<<" fen: "<<fens[i]<<endl;
	}

}

Eval WorkerThread::search(Pos &p, Depth depth, Eval alpha, Eval beta) {
	if (!parent->searching) return 0;
	nodes++;

	if (depth <= 0) return qsearch(p, alpha, beta);
	if (p.hm_clock >= 4) {
		if (p.hm_clock == 50) return 0;
		if (p.insufficientMaterial()) return 0;
		if (p.oneRepetition(root_pos.hashkey_log.size())) return 0;
		if (p.threeRepetitions()) return 0;
	}

	if (alpha >= MINMATE && alpha != INF) {
		alpha++;
		if (alpha >= beta) return beta-1;
	}

	bool found = false;
	TTEntry* entry = parent->tt.probe(p.hashkey, found);
	Move entry_move = entry->move;

	if (found && parent->useHashTable) {
		if (entry->depth >= depth) {
			if (entry->bound == EXACT) return entry->eval;
			else if (entry->bound == UB && entry->eval < beta) beta = entry->eval;
			else if (entry->bound == LB && entry->eval > alpha) alpha = entry->eval;
			if (alpha >= beta) return beta;
		}
	}

	vector<Move> moves = getLegalMoves(p);

	if (moves.size() == 0) {
		if (p.isInCheck()) return -INF;
		else return 0;
	}

	moves = order(*parent, p, moves, found ? entry_move : MOVENONE);

	Eval besteval = -INF;
	Move bestmove = moves[0];

	bool isfutilityPruningNode = parent->futilityPruning && depth == 1 && (evalPos(p, alpha, beta) + parent->futilityMargin < alpha);

	for (int i = 0; i < moves.size(); i++) {
		Move &move = moves[i];
		if (!isfutilityPruningNode || isCapture(move) || isPromotion(move) || p.causesCheck(move)) {
			p.makeMove(move);

			Eval eval = -search(p, depth - (i >= (found ? 1 : 3) ? (i >= 8 ? 3 : 2) : 1), -beta, -max(alpha, besteval));

			p.undoMove();

			if (!parent->searching) return 0;
			
			if (eval > besteval) {
				besteval = eval;
				bestmove = move;
				
				if (besteval >= beta) {
					Move& prev_move = p.move_log.back();
					parent->cm_hueristic[p.getPieceAt(p.notturn, getTo(prev_move))][getTo(prev_move)] = move;
					parent->hist_hueristic[p.getPieceAt(p.turn, getFrom(move))][getTo(move)]++;
					break;
				}
			}
		}
	}
	if (!parent->searching) return 0;

	if (besteval <= alpha) 	  entry->save(p.hashkey, besteval, UB   , depth, bestmove, parent->tt.gen);
	else if (besteval < beta) entry->save(p.hashkey, besteval, EXACT, depth, bestmove, parent->tt.gen);
	else if (besteval == INF) entry->save(p.hashkey, besteval, EXACT, depth, bestmove, parent->tt.gen);
	else 					  entry->save(p.hashkey, besteval, LB   , depth, bestmove, parent->tt.gen);

	if (besteval > MINMATE) besteval--;
	return besteval;
}

Eval WorkerThread::qsearch(Pos &p, Eval alpha, Eval beta) {
	if (!parent->searching) return 0;
	nodes++;

    Eval stand_pat = evalPos(p, alpha, beta);
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;

    vector<Move> moves = getLegalMoves(p);
    
    for (Move& m : moves) {
        if (isCapture(m) || isPromotion(m)) {
            p.makeMove(m);
            alpha = max(alpha, (Eval)-qsearch(p, -beta, -alpha));
            p.undoMove();
			if (!parent->searching) return 0;
            if (alpha >= beta) {
                return beta;
            }
        }
    }
    return alpha;
}

/*
	static const vector<pair<string, string>> puzzles = {
		{"1Q6/1pR2p1p/3pkp2/5p2/q3P3/3P4/Pr5P/7K w - - 6 25", "b8c8"},
		{"r4rk1/ppp1qppp/4bn2/6B1/4n2Q/2P2N2/PP2PPBP/3RK2R w K - 5 15", "h4e4"},
		{"6k1/5r1p/p5p1/1p6/4q1PP/1Q1p1NK1/PP3P2/8 w - - 3 33", "b3f7"},
		{"r2q3k/pp2p2p/3p4/5b2/3Pn3/1P3N2/P1P2Pr1/2KR1Q1R w - - 0 22", "f1g2"},
		{"2k5/p1p5/5r2/2PP2n1/P3r1p1/4N1P1/5P2/R1R3K1 b - - 3 43", "g5h3"},
		{"1k4r1/n1p3q1/Qb1p4/3P2rp/1PN4N/8/4R1P1/R6K b - - 11 40", "g5g4"},
		{"5r1k/p5p1/4Q2p/3N4/3q4/5r2/PPP1R2P/1R5K b - - 5 27", "f3f1"},
		{"r5k1/pp4pp/2p3q1/3p1r2/3Pb3/N1PBp1P1/PPQ4P/R3R1K1 b - - 6 22", "f5f2"},
		{"8/3P4/n2K2kp/2p3nN/1b6/2p1p1P1/8/3B4 w - - 4 3", "d1c2"},
	};
*/

mutex depth_mtx;

void Search::go() {
	thread(&Search::manager, &(*this)).detach();
}

void Search::stop() {
	searching = false;
}

void Search::manager() {
	search_start = get_current_ms();

	searching = true;
	min_thread_depth = 0;
	max_thread_depth = 0;

	for (int i = 0; i < 6; i++) {
		for (int ii = 0; ii < 64; ii++) {
			hist_hueristic[i][ii] = 0;
		}
	}

	vector<thread> threads;
	vector<WorkerThread> workers;
	for (int i = 0; i < num_threads; i++) workers.push_back(WorkerThread(*this));
	for (int i = 0; i < num_threads; i++) thread(&WorkerThread::start, &workers[i]).detach();

	Depth last_depth_printed = 0;
	BB nodes_at_last = 0;
	Timestamp time_of_last = get_current_ms();
	
	TTEntry* entry;

	while (searching) {
		bool found = false;
		entry = tt.probe(root_pos.hashkey, found);

		Timestamp max_time;
		if (root_pos.turn == WHITE) max_time = min((Timestamp)5000, wtime/2);
		else max_time = min((Timestamp)5000, btime/2);

		//cout<<to_string(get_time_diff(search_start))<<" < "<<to_string(max_time)<<endl;
		if (get_time_diff(search_start) > max_time && !ponder && !infinite) {
			searching = false;
		}
		
		if (last_depth_printed != min_thread_depth) {

			BB nodes = 0;
			for (WorkerThread& worker : workers) nodes += worker.nodes;
				
			last_depth_printed = min_thread_depth;

			cout<<"info";
			cout<<" depth "<<to_string(min_thread_depth);
			cout<<" seldepth "<<to_string(max_thread_depth);
			cout<<" score "<<evalToString(entry->eval);
			cout<<" time "<<to_string(get_time_diff(search_start));
			cout<<" nodes "<<to_string(nodes);
			if (get_time_diff(time_of_last)) cout<<" nps "<<to_string((nodes-nodes_at_last)*1000/get_time_diff(time_of_last));
			cout<<" pv "; print(tt.getPV(root_pos));
			cout<<endl;

			nodes_at_last = nodes;
			time_of_last = get_current_ms();
		}
	}

	BB nodes = 0;
	for (WorkerThread& worker : workers) nodes += worker.nodes;

	cout<<"info";
	cout<<" depth "<<to_string(min_thread_depth);
	cout<<" seldepth "<<to_string(max_thread_depth);
	cout<<" score "<<evalToString(entry->eval);
	cout<<" time "<<to_string(get_time_diff(search_start));
	cout<<" nodes "<<to_string(nodes);
	if (get_time_diff(time_of_last)) cout<<" nps "<<to_string((nodes-nodes_at_last)*1000/get_time_diff(time_of_last));

	vector<Move> pv = tt.getPV(root_pos);

	cout<<" pv "; print(tt.getPV(root_pos));
	cout<<endl;

	cout<<"bestmove "<<getSAN(pv[0])<<endl;

	if (root_pos.turn == WHITE) wtime += -get_time_diff(search_start) + winc;
	else 						btime += -get_time_diff(search_start) + binc;
}

Search::Search(Pos p) {
	root_pos = p;
	root_moves = getLegalMoves(root_pos);
}

WorkerThread::WorkerThread(Search& search) {
	parent = &search;
	root_pos = search.root_pos;
}

void WorkerThread::start() {
	if (root_pos.hm_clock >= 4) {
		if (root_pos.hm_clock == 50) return;
		if (root_pos.insufficientMaterial()) return;
		if (root_pos.oneRepetition(root_pos.hashkey_log.size())) return;
		if (root_pos.threeRepetitions()) return;
	}

	
	while (parent->searching) {
		parent->min_thread_depth = max(root_depth, (Depth)parent->min_thread_depth);
		root_depth = ++parent->max_thread_depth;

		Pos p = root_pos;

		bool found = false;
		TTEntry* entry = parent->tt.probe(p.hashkey, found);
		Move entry_move = entry->move;

		vector<Move> moves = getLegalMoves(p);
		moves = order(*parent, p, moves, found ? entry_move : MOVENONE);
		Eval besteval = -INF;
		Move bestmove = moves[0];
		for (int i = 0; i < moves.size(); i++) {
			Move &move = moves[i];
			p.makeMove(move);

			Eval eval = -search(p, root_depth - (i >= (found ? 1 : 3) ? (i >= 8 ? 3 : 2) : 1), -INF, -besteval);

			p.undoMove();

			if (!parent->searching) return;
			
			if (eval > besteval) {
				besteval = eval;
				bestmove = move;
			}
		}
		
		if (!parent->searching) return;

		entry->save(p.hashkey, besteval, EXACT, root_depth, bestmove, parent->tt.gen);

		parent->root_bestmove = bestmove;
	}
}
