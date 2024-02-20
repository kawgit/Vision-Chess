#include <cassert>

#include "movepicker.h"
#include "movegen.h"

MovePicker::MovePicker(Pos* pos_, const Move tt_move_, const History* history_) {

    // pos, tt_move, history

    pos     = pos_;
    tt_move = tt_move_;
    history = history_;

    // moves, scores, num_left, num_left_in_stage

    num_left_in_stage = 0;

    moves = movegen::generate<LEGAL>(*pos);

    num_left = moves.size();

    if (!num_left)
        return;

    scores = std::vector<Score>(moves.size(), SCORE_UNKNOWN);


    // from_to_counter, piece_to_counter, piece_from_counter

    const Move   last_move   = pos->slice->move;
    const Square from_square = move::from_square(last_move);
    const Square to_square   = move::to_square  (last_move);
    const Piece  moving      = pos->slice->moving;
    
    HistorySlice* history_slice = history->history_slice;

    from_to_counter    = history_slice->from_to   [pos->turn()][from_square][to_square  ];
    piece_to_counter   = history_slice->piece_to  [pos->turn()][moving     ][to_square  ];
    piece_from_counter = history_slice->piece_from[pos->turn()][moving     ][from_square];

    pos->update_attacks(WHITE);
    pos->update_attacks(BLACK);

    // undefended_bbs

    undefended_bbs[WHITE] = pos->pieces(WHITE) & ~pos->attacked_by(WHITE);
    undefended_bbs[BLACK] = pos->pieces(BLACK) & ~pos->attacked_by(BLACK);

    // unsafe_bbs

    const BB hanging_destinations = pos->attacked_by(pos->notturn()) & ~pos->attacked_by(pos->turn());
    BB cum_attacks = BB_EMPTY;

    unsafe_bbs[PAWN] = hanging_destinations;
    cum_attacks |= pos->attacked_by(pos->notturn(), PAWN);

    unsafe_bbs[KNIGHT] = hanging_destinations | cum_attacks;
    unsafe_bbs[BISHOP] = hanging_destinations | cum_attacks;
    cum_attacks |= pos->attacked_by(pos->notturn(), KNIGHT);
    cum_attacks |= pos->attacked_by(pos->notturn(), BISHOP);

    unsafe_bbs[ROOK] = hanging_destinations | cum_attacks;
    cum_attacks |= pos->attacked_by(pos->notturn(), ROOK);
    
    unsafe_bbs[QUEEN] = hanging_destinations | cum_attacks;
    cum_attacks |= pos->attacked_by(pos->notturn(), KING) & ~pos->attacked_by(pos->turn());

    unsafe_bbs[KING] = pos->attacked_by(pos->notturn());

    // tempo_bbs

    const BB occupied = pos->pieces() & ~pos->pieces(pos->notturn(), KING);

    BB cum_victims = undefended_bbs[pos->notturn()];

    tempo_bbs[KING]   = attacks::kings  (cum_victims          ) &~ cum_attacks;
    cum_victims |= pos->pieces(pos->notturn(), KING);

    tempo_bbs[QUEEN]  = attacks::queens (cum_victims, occupied) &~ cum_attacks;       
    cum_victims |= pos->pieces(pos->notturn(), QUEEN);

    tempo_bbs[ROOK]   = attacks::rooks  (cum_victims, occupied) &~ unsafe_bbs[QUEEN]; 
    cum_victims |= pos->pieces(pos->notturn(), ROOK);

    tempo_bbs[BISHOP] = attacks::bishops(cum_victims, occupied) &~ unsafe_bbs[ROOK];
    tempo_bbs[KNIGHT] = attacks::knights(cum_victims          ) &~ unsafe_bbs[ROOK];
    cum_victims |= pos->pieces(pos->notturn(), BISHOP);
    cum_victims |= pos->pieces(pos->notturn(), KNIGHT);
    
    tempo_bbs[PAWN]   = attacks::pawns  (cum_victims, pos->notturn()) &~ unsafe_bbs[KNIGHT];

    // check_bbs

    enemy_king_square = lsb(pos->pieces(pos->notturn(), KING));

    check_bbs[KING]   = BB_EMPTY;
    check_bbs[QUEEN]  = attacks::queen (enemy_king_square, occupied     );
    check_bbs[ROOK]   = attacks::rook  (enemy_king_square, occupied     );
    check_bbs[BISHOP] = attacks::bishop(enemy_king_square, occupied     );
    check_bbs[KNIGHT] = attacks::knight(enemy_king_square               );
    check_bbs[PAWN]   = attacks::pawn  (enemy_king_square, pos->notturn());

    // critical_situation

    critical_situation = undefended_bbs[pos->turn()] & pos->attacked_by(pos->notturn());
}

template<MovePickerStage STAGE>
Score MovePicker::score_move(const Move move) {
    
    constexpr Score capture_values  [N_PIECES + 1] = { 10, 30, 30, 50, 90, 0, 0 };
    constexpr Score promotion_values[N_PIECES    ] = { 0, 10, -50, -50, 60, 0 };

    if constexpr (STAGE == STAGE_TT_COUNTERS) {
        return SCORE_UNKNOWN + 10 * (move == tt_move) + 3 * (move == from_to_counter) + 1 * (move == piece_from_counter) + 2 * (move == piece_to_counter);
    }

    if constexpr (STAGE == STAGE_LOUDS) {
        
        const Square from_square   = move::from_square(move);
        const Square to_square     = move::to_square  (move);
        const Square victim_square = move::capture_square(move);
        const Piece  moving        = pos->piece_on(from_square);
        const Piece  victim        = pos->piece_on(victim_square);

        const BB post_occupied = (pos->pieces() & ~(bb_of(from_square) | bb_of(victim_square))) | bb_of(to_square);
        const bool is_revealed_check = (attacks::bishop(enemy_king_square, post_occupied) & pos->pieces(pos->turn(), BISHOP))
                                     | (attacks::rook  (enemy_king_square, post_occupied) & pos->pieces(pos->turn(), ROOK  ));

        const bool is_check     = (check_bbs[moving] & bb_of(to_square)) || is_revealed_check;
        const bool is_capture   = move::is_capture(move);
        const bool is_promotion = move::is_promotion(move);
        const bool is_hanging   = unsafe_bbs[moving] & bb_of(from_square);

        if (!is_check && !is_capture && !is_promotion && !is_hanging) {
            return SCORE_UNKNOWN;
        }

        const bool is_unsafe  = unsafe_bbs[moving] & bb_of(to_square);
        const bool is_tempo   = tempo_bbs [moving] & bb_of(to_square);

        const Score capture_value   = capture_values[victim] - capture_values[moving] * is_unsafe;
        const Score promotion_value = move::is_promotion(move) ? promotion_values[move::promotion_piece(move)] : 0;
        const Score quiet_value     = 10 * is_tempo + capture_values[moving] * is_hanging;

        return 10000 * (is_check || is_capture || is_promotion || is_hanging) + capture_value + promotion_value + quiet_value;
    }

    if constexpr (STAGE == STAGE_QUIETS) {
        
        const Square from_square = move::from_square(move);
        const Square to_square   = move::to_square  (move);
        const Piece  moving      = pos->piece_on(from_square);
        const Piece  victim      = pos->piece_on(move::capture_square(move));

        const bool is_unsafe  = unsafe_bbs[moving] & bb_of(to_square);
        const bool is_tempo   = tempo_bbs [moving] & bb_of(to_square);
        const bool is_hanging = unsafe_bbs[moving] & bb_of(from_square);

        const Score quiet_value   = 100 * is_tempo + capture_values[moving] * is_hanging;
        // const Score history_value = critical_situation ? 0 : history->score_move(from_square, to_square, moving, pos->turn());

        return std::max(quiet_value, Score(SCORE_MIN));
    }

}

template<MovePickerStage STAGE>
void MovePicker::score_moves() {

    num_left_in_stage = 0;
    
    for (size_t i = 0; i < moves.size(); i++) {
        
        if (scores[i] == SCORE_UNKNOWN)
            scores[i] = score_move<STAGE>(moves[i]);

        num_left_in_stage += (scores[i] >= SCORE_MIN);
    }

}

Move MovePicker::pop() {

    if (num_left == 0)
        return MOVE_NONE;

    while (num_left_in_stage == 0)
        next_stage();

    num_left--;
    num_left_in_stage--;

    size_t best_index = 0;

    for (size_t i = 1; i < moves.size(); i++)
        if (scores[i] > scores[best_index])
            best_index = i;

    scores[best_index] = SCORE_USED;

    return moves[best_index];

}

void MovePicker::next_stage() {

    assert(num_left_in_stage == 0);
    assert(num_left != 0);
    assert(stage != STAGE_QUIETS);

    stage++;

    switch (stage) {
        
        case STAGE_TT_COUNTERS:

            score_moves<STAGE_TT_COUNTERS>();

            break;
        
        case STAGE_LOUDS:

            score_moves<STAGE_LOUDS>();

            break;
        
        case STAGE_QUIETS:

            score_moves<STAGE_QUIETS>();

            break;

    }


}

bool MovePicker::has_move() {
    return num_left != 0;
}
