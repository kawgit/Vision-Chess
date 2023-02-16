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

    vector<Score> unsorted_boring_moves;
    vector<Score> unsorted_bad_moves;
    if (!for_qsearch) {
        unsorted_boring_moves.reserve(unsorted_moves.size());
        unsorted_bad_moves.reserve(unsorted_moves.size());
    }

    vector<Move> sorted_moves;
    vector<Score> sorted_scores;
    sorted_moves.reserve(unsorted_moves.size());
    sorted_scores.reserve(unsorted_moves.size());

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
            //     score += sea_gain(pos, move, -200);
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
            insert_to_sorted(move, score, sorted_moves, sorted_scores, 0);
        }
        else if (!for_qsearch) {
            if (score == 0) {
                unsorted_boring_moves.push_back(move);
            }
            else {
                unsorted_bad_moves.push_back(move);
            }
        }
    }

    num_good = sorted_moves.size();
    num_boring = unsorted_boring_moves.size();
    num_bad = unsorted_bad_moves.size();

    if (for_qsearch) return sorted_moves;

    if (false && pos.last_move() != MOVE_NONE) {
        // BB our_threatened = get_threatened(pos, pos.turn);
        // BB their_threatened = get_threatenable(pos, pos.notturn);
        BB our_threatened = get_piece_atk(pos.last_from_piece(), pos.last_from(), pos.notturn, pos.ref_occ()) & pos.ref_occ();
        for (Move move : unsorted_boring_moves) {
            // sorted_moves.push_back(unsorted_boring_moves[i]);
            // sorted_scores.push_back(0);
            // si.get_hist(pos, unsorted_boring_moves[i]);
            Score score = 0;
            BB to_atk = get_piece_atk(pos.ref_mailbox(pos.turn, get_from(move)), get_to(move), pos.turn, pos.ref_occ());
            if (to_atk & our_threatened) score += 10;
            else if (get_BB(get_from(move)) & our_threatened) {
                if (pos.ref_mailbox(pos.turn, get_from(move)) > pos.last_from_piece()
                    || !(pos.ref_atk(pos.turn) & get_BB(get_from(move)))) {
                    score += 20;
                }
                else {
                    score += 5;
                }
            }
            insert_to_sorted(move, score, sorted_moves, sorted_scores, num_good);
        }
    }
    else {
        for (Move move : unsorted_boring_moves) {
            sorted_moves.push_back(move);
        }
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
