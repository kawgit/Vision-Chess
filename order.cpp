#include "types.h"
#include "pos.h"
#include "search.h"
#include "bits.h"
#include "eval.h"
#include "tuner.h"
#include "movegen.h"
#include <vector>

void insert_to_sorted(Move move, Score score, vector<Move>& moves, vector<Score>& scores, int lb) {
    moves.push_back(move);
    scores.push_back(score);
    for (int i = scores.size() - 2; i >= lb; i--) {
        if (scores[i] < score) {
            moves[i+1] = moves[i];
            scores[i+1] = scores[i];
        }
        else {
            moves[i+1] = move;
            scores[i+1] = score;
            return;
        }
    }
    moves[lb] = move;
    scores[lb] = score;
}

vector<Move> order(vector<Move>& unsorted_moves, Pos& pos, ThreadInfo& ti, SearchInfo& si, int& num_good, int& num_boring, int& num_bad, bool for_qsearch) {
    Move counter_move = si.get_cm(pos);

    bool found = false;
	TTEntry* entry = si.tt.probe(pos.ref_hashkey(), found);
	Move entry_move = found ? entry->get_move() : MOVE_NONE;

    vector<Score> unsorted_good_scores;
    vector<Score> unsorted_good_moves;
    vector<Score> unsorted_boring_scores;
    vector<Score> unsorted_boring_moves;
    vector<Score> unsorted_bad_scores;
    vector<Score> unsorted_bad_moves;

    unsorted_good_scores.reserve(unsorted_moves.size());
    unsorted_good_moves.reserve(unsorted_moves.size());
    if (!for_qsearch) {
        unsorted_boring_scores.reserve(unsorted_moves.size());
        unsorted_boring_moves.reserve(unsorted_moves.size());
        unsorted_bad_scores.reserve(unsorted_moves.size());
        unsorted_bad_moves.reserve(unsorted_moves.size());
    }

    pos.update_atks();
    BB occ = pos.ref_occ();
    BB turn_atk = pos.ref_atk(pos.turn);
    BB notturn_atk = pos.ref_atk(pos.notturn);

    bool found_huer_response = false;
    for (Move& move : unsorted_moves) {
        Score score = 0;

        if (move == entry_move) score = 1000001;
        else if (move == counter_move) score = 1000000;
        else {
            // if (!(for_qsearch || is_ep(move) || is_king_castle(move) || is_queen_castle(move) || is_promotion(move))) {
            //     score += sea_gain(pos, move, -INF);
            // }

            if (is_capture(move)) score += 100;
            if (is_promotion(move)) {
                switch (get_promotion_type(move)) {
                    case QUEEN:
                        score += 900;
                        break;
                    case KNIGHT:
                        score -= 500; // usually only wanted because it comes with check, which is accounted for elsewhere
                    case BISHOP:
                    case ROOK:
                        score -= 1000; // should practically garaunteed to not be best move
                        break;
                    default:
                        assert(false);
                        break;

                }
            }
            if (pos.causes_check(move)) score += 1000;
        }

        if (score > 0) {
            unsorted_good_scores.push_back(score);
            unsorted_good_moves.push_back(move);
        }
        else if (!for_qsearch) {
            if (score < 0) {
                unsorted_bad_scores.push_back(score);
                unsorted_bad_moves.push_back(move);
            }
            else {
                unsorted_boring_scores.push_back(score);
                unsorted_boring_moves.push_back(move);
            }
        }
    }

    num_good = unsorted_good_moves.size();
    num_boring = unsorted_boring_moves.size();
    num_bad = unsorted_bad_moves.size();

    vector<Move> sorted_moves;
    vector<Score> sorted_scores;
    sorted_moves.reserve(unsorted_moves.size());
    sorted_scores.reserve(unsorted_moves.size());

    for (int i = 0; i < unsorted_good_moves.size(); i++) {
        // sorted_moves.push_back(unsorted_good_moves[i]);
        insert_to_sorted(unsorted_good_moves[i], unsorted_good_scores[i], sorted_moves, sorted_scores, 0);
    }

    if (for_qsearch) return sorted_moves;
    
    for (int i = 0; i < unsorted_boring_moves.size(); i++) {
        sorted_moves.push_back(unsorted_boring_moves[i]);
        // sorted_scores.push_back(0);
        // insert_to_sorted(unsorted_boring_moves[i], unsorted_boring_scores[i], sorted_moves, sorted_scores, num_good);
    }
    
    for (int i = 0; i < unsorted_bad_moves.size(); i++) {
        sorted_moves.push_back(unsorted_bad_moves[i]);
        // sorted_scores.push_back(-1);
        // insert_to_sorted(unsorted_bad_moves[i], unsorted_bad_scores[i], sorted_moves, sorted_scores, num_good + num_boring);
    }

    // early game bonuses
    /*
    if (get_early_weight(pos, pos.turn) > .6) {
        for (int i = 0; i < unsorted_moves.size(); i++) {
            if (unsorted_scores[i] >= 0) {
                Move move = unsorted_moves[i];
                Square from = get_from(move);
                if (pos.ref_mailbox(pos.turn, from) != ROOK) {
                    Rank rank = rank_of(from);
                    if (rank == RANK_1 || rank == RANK_2) {
                        unsorted_scores[i] += 10;
                    }
                }

                if (is_king_castle(move) || is_queen_castle(move)) {
                    unsorted_scores[i] += 20;
                }
            }
        }
    }
    */

    // end game bonuses
    /*
    if (get_late_weight(pos, opp(pos.turn)) > .6) {
        BB passed_pawns = pos.passed_pawns(pos.turn);
        for (int i = 0; i < unsorted_moves.size(); i++) {
            if (unsorted_scores[i] >= 0) {
                Square from = get_from(unsorted_moves[i]);
                
                // push passed pawns
                if (get_BB(from) & passed_pawns) {
                    unsorted_scores[i] += 10;
                }
            }
        }
    }
    */

    // for (int i = 0; i < sorted_moves.size(); i++) {
    //     cout << to_san(sorted_moves[i]) << " " << to_string(sorted_scores[i]) << endl;
    // }

    assert(sorted_moves.size() == unsorted_moves.size());

    return sorted_moves;
}
