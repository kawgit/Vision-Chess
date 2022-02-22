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

#define REDUCTION(d, i, f) ((d)-1)

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

Eval search(Pos &p, Depth depth, SearchInfo* searchInfo) {
	ThreadInfo threadInfo(p);
	return search(p, depth, -INF, INF, threadInfo, searchInfo);
}

Eval search(Pos &p, Depth depth, Eval alpha, Eval beta, ThreadInfo& threadInfo, SearchInfo* searchInfo)
{

	if (!threadInfo.searching) return 0;
	threadInfo.nodes++;

	if (p.hm_clock >= 4) {
		if (p.hm_clock == 50) return 0;
		if (p.insufficientMaterial()) return 0;
		if (p.oneRepetition(threadInfo.root_ply)) return 0;
		if (p.threeRepetitions()) return 0;
	}

	if (depth <= 0) return qsearch(p, alpha, beta, threadInfo, searchInfo);

	if (alpha >= MINMATE && alpha != INF) {
		alpha++;
		if (alpha >= beta) return beta-1;
	}

	bool found = false;
	TTEntry* entry = (searchInfo != nullptr && searchInfo->tt != nullptr) ? searchInfo->tt->probe(p.hashkey, found) : nullptr;
	Move entry_move = found ? entry->move : MOVENONE;

	if (found) {
		if (entry->eval >= MINMATE && entry->eval > alpha) {
			alpha = entry->eval;
			if (alpha >= beta) return beta;
		}
		
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
	
	moves = order(searchInfo, p, moves, found ? entry_move : MOVENONE);

	Eval besteval = -INF;
	Move bestmove = moves[0];


	for (int i = 0; i < moves.size(); i++) {
		Move &move = moves[i];
		p.makeMove(move);
		
		Eval eval = -search(p, REDUCTION(depth, i, found), -beta, -max(alpha, besteval), threadInfo, searchInfo);

		p.undoMove();
		
		if (eval > besteval) {
			if (!threadInfo.searching) return 0;

			besteval = eval;
			bestmove = move;
			
			if (besteval >= beta) {
				if (searchInfo != nullptr) {
					Move& prev_move = p.move_log.back();
					searchInfo->cm_hueristic[p.getPieceAt(p.notturn, getTo(prev_move))][getTo(prev_move)] = move;
					searchInfo->hist_hueristic[p.getPieceAt(p.turn, getFrom(move))][getTo(move)]++;
				}
				break;
			}
		}
	}

	if (!threadInfo.searching) return 0;
	
	if (besteval > MINMATE) besteval--;

	if (searchInfo != nullptr && searchInfo->tt != nullptr) {
		if (besteval <= alpha) 	  entry->save(p.hashkey, besteval, UB   , depth, bestmove, searchInfo->tt->gen);
		else if (besteval < beta) entry->save(p.hashkey, besteval, EXACT, depth, bestmove, searchInfo->tt->gen);
		else if (besteval == INF) entry->save(p.hashkey, besteval, EXACT, depth, bestmove, searchInfo->tt->gen);
		else 					  entry->save(p.hashkey, besteval, LB   , depth, bestmove, searchInfo->tt->gen);
	}

	return besteval;
}

Eval qsearch(Pos &p, Eval alpha, Eval beta, ThreadInfo& threadInfo, SearchInfo* searchInfo)
{
	if (!threadInfo.searching) return 0;
	threadInfo.nodes++;

    Eval stand_pat = evalPos(p, alpha, beta);
    if (stand_pat >= beta) return beta;
    if (alpha < stand_pat) alpha = stand_pat;

    vector<Move> moves = getLegalMoves(p);

	if (moves.size() == 0) {
		if (p.isInCheck()) return -INF;
		else return 0;
	}
    
    for (Move& m : moves) {
        if (isCapture(m) || isPromotion(m)) {
            p.makeMove(m);
            alpha = max(alpha, (Eval)-qsearch(p, -beta, -alpha, threadInfo, searchInfo));
            p.undoMove();
            if (alpha >= beta) {
                return beta;
            }
        }
    }
	
    return alpha;
}

inline unsigned int mvvlva(Piece attacker, Piece victim) {
    static const unsigned int table[6][6] = { //attacker, victim
        {6000, 20220, 20250, 20400, 20800, 26900},
        {4770,  6000, 20020, 20170, 20570, 26670},
        {4750,  4970,  6000, 20150, 20550, 26650},
        {4600,  4820,  4850,  6000, 20400, 26500},
        {4200,  4420,  4450,  4600,  6010, 26100},
        {3100,  3320,  3350,  3500,  3900, 26000},
    };
    return table[attacker][victim];
}

vector<Move> order(SearchInfo* searchInfo, Pos& pos, vector<Move> unsorted_moves, Move entry_move) {
    Move prev_move = (pos.move_log.size() ? pos.move_log.back() : MOVENONE);
    Move cm = (searchInfo != nullptr && pos.move_log.size()) ? searchInfo->cm_hueristic[pos.getPieceAt(pos.notturn, getTo(prev_move))][getTo(prev_move)] : MOVENONE;

    vector<unsigned int> unsorted_scores;
    unsorted_scores.reserve(unsorted_moves.size());
    for (Move& move : unsorted_moves) {
        unsigned int score = 0;
        if (move == entry_move) score = -1;
        else {
			if (move == cm) score += 10000;
            Piece fromPiece = pos.getPieceAt(pos.turn, getFrom(move));
            if (isCapture(move)) {
                if (!isEp(move)) score += mvvlva(fromPiece, pos.getPieceAt(pos.notturn, getTo(move)));
                else score += mvvlva(fromPiece, PAWN);
            }
            if (isPromotion(move)) score += mat_points[getPromotionType(move)]*12;
            if (fromPiece >= KNIGHT && pos.causesCheck(move)) score += 10000 + fromPiece;
			score += searchInfo->hist_hueristic[fromPiece][getTo(move)];
        }
        unsorted_scores.push_back(score);
    }


    vector<Move> sorted_moves;
    vector<unsigned int> sorted_scores;
    sorted_moves.reserve(unsorted_moves.size());
    sorted_scores.reserve(unsorted_moves.size());

    for (int j = 0; j < unsorted_moves.size(); j++) {
        Move& move = unsorted_moves[j];
        unsigned int& score = unsorted_scores[j];

        sorted_moves.push_back(move);
        sorted_scores.push_back(score);
        int i = sorted_moves.size()-1;
        while (i != -1) {
            i--;
            if (sorted_scores[i] < score) {
                sorted_moves[i+1] = sorted_moves[i];
                sorted_scores[i+1] = sorted_scores[i];
            }
            else break;
        }
        sorted_moves[i+1] = move;
        sorted_scores[i+1] = score;
    }

    return sorted_moves;
}
