#include <iostream>

#include "attacks.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "pos.h"
#include "search.h"
#include "timer.h"
#include "tt.h"
#include "uci.h"
#include "util.h"

template BB perft<true >(Pos& pos, Depth depth);
template BB perft<false>(Pos& pos, Depth depth);

template<bool DIVIDE>
BB perft(Pos& pos, Depth depth) {

	if (depth == 0) {
		if constexpr (DIVIDE)
			std::cout << "divide on depth zero, total: 1" << std::endl;
		return 1;
	}

	Timestamp start = 0;
	if constexpr (DIVIDE)
		start = get_current_ms();

	BB count = 0;
	std::vector<Move> moves = movegen::generate<LEGAL>(pos);

	if (depth == 1 && !DIVIDE)
		return moves.size();

	for (Move move : moves) {

		BB hashkey_before = pos.hashkey();

		if constexpr (DIVIDE)
			std::cout << move_to_string(move) << " ";
		
		pos.do_move(move);
		BB n = perft<false>(pos, depth - 1);
		
		if constexpr (DIVIDE)
			std::cout << std::to_string(n) << std::endl;
		
		pos.undo_move();
		
		assert(hashkey_before == pos.hashkey());

		count += n;
	}

	if constexpr (DIVIDE) {
		std::cout << "total: " << std::to_string(count) << std::endl;
		std::cout << "time: " << std::to_string(get_time_diff(start)) << " ms" << std::endl;
		std::cout << "knps: " << std::to_string(count / (get_time_diff(start) + 1)) << std::endl;
	}

	return count;
}


template<NodeType NODETYPE>
Eval Thread::search(Depth depth, Eval alpha, Eval beta) {

	if (requested_state != ACTIVE)
		return 0;
	
	nodes++;

	constexpr bool is_qs   = NODETYPE == QSNODE;
	constexpr bool is_root = NODETYPE == ROOT;
	constexpr bool is_pv   = NODETYPE == ROOT || NODETYPE == PVNODE;

	if constexpr (!is_qs) {
        if (depth <= 0)
		    return search<QSNODE>(depth, alpha, beta);
    }
    else {
        if (depth <= -16)
            return evaluator.evaluate(pos);
    }

	bool found = false;
	TTEntry* entry = tt->probe(pos.hashkey(), found);

	Move  tt_move  = found ? entry->move : MOVE_NONE;
	Eval  tt_eval  = entry->eval;
	Depth tt_depth = entry->depth;
	Bound tt_bound = entry->get_bound();

	// if (found && tt_depth >= depth) {
		
	// 	if (tt_bound == EXACT)
	// 		return tt_eval;

	// 	if (tt_bound == UB && tt_eval < beta)
	// 		beta = tt_eval;

	// 	if (tt_bound == LB && tt_eval > alpha)
	// 		alpha = tt_eval;

	// 	if (alpha >= beta)
	// 		return beta;
	// }

	// if constexpr (NODETYPE == CUTNODE) {
	// 	constexpr Eval alpha_reduction = 100;
	// 	Eval eval = search<CUTNODE>(depth - 2, alpha - alpha_reduction, alpha - (alpha_reduction - 1));
	// 	if (eval <= alpha - alpha_reduction) return alpha - alpha_reduction;
	// }

	MovePicker mp(&pos, tt_move, &history);

	Move best_move = MOVE_NONE;
	Eval best_eval = EVAL_MIN;

	if constexpr (is_qs) {
        best_eval = evaluator.evaluate(pos);
        if (best_eval >= beta) {
		    tt->save(entry, best_move, best_eval, depth, pos.hashkey(), LB);
            return best_eval;
        }
    }
    else if (!mp.has_move()) {
		if (pos.checkers()) {
			best_eval = -EVAL_CHECKMATE;
		}
		else {
			best_eval = 0;
		}
	}

	while (mp.has_move() && (!is_qs || pos.checkers() || mp.stage != STAGE_QUIETS)) {

		Move move = mp.pop();

		bool is_pv_child = is_pv && (tt_move == MOVE_NONE || tt_move == move);

		do_move(move);

		Eval eval;
		
		if (is_pv_child)
			eval = -search<PVNODE >(depth - (1 + (mp.stage == STAGE_QUIETS)), -beta, -std::max(alpha, best_eval));
		else if (is_qs)
			eval = -search<QSNODE >(depth - 1, -beta, -std::max(alpha, best_eval));
		else
			eval = -search<CUTNODE>(depth - (1 + 2 * (mp.stage == STAGE_QUIETS)), -beta, -std::max(alpha, best_eval));

		undo_move();

		if (eval > best_eval) {
			best_move = move;
			best_eval = eval;

			if (best_eval >= beta)
				break;
		}

	}

	if (requested_state != ACTIVE)
		return 0;

	const Bound bound = is_qs ?              LB :
                        best_eval <= alpha ? UB :
						best_eval >= beta  ? LB :
											 EXACT;
	
	if constexpr (is_root || is_pv)
		tt->forcesave(entry, best_move, best_eval, depth, pos.hashkey(), bound);
	else
		tt->save     (entry, best_move, best_eval, depth, pos.hashkey(), bound);

	// if (bound != UB) {
	// 	const Score bonus = 10 * depth * depth + 50 * depth + 20;
	// 	history.update_bonus(best_move, pos, bonus);
	// }

	return best_eval;

}


template Eval Thread::search<ROOT   >(Depth depth, Eval alpha, Eval beta);
template Eval Thread::search<PVNODE >(Depth depth, Eval alpha, Eval beta);
template Eval Thread::search<CUTNODE>(Depth depth, Eval alpha, Eval beta);
template Eval Thread::search<QSNODE >(Depth depth, Eval alpha, Eval beta);
