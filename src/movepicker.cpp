#include <cassert>

#include "movegen.h"
#include "movepicker.h"

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

    scores = std::vector<Score>(moves.size());


    // from_to_counter, piece_to_counter, piece_from_counter

    const Move   last_move   = pos->slice->move;
    const Square from_square = move::from_square(last_move);
    const Square to_square   = move::to_square  (last_move);
    const Piece  moving      = pos->slice->moving;
    
    HistorySlice* history_slice = history->history_slice;

    from_to_counter    = history_slice->from_to   [pos->turn()][from_square][to_square  ];
    piece_to_counter   = history_slice->piece_to  [pos->turn()][moving     ][to_square  ];
    piece_from_counter = history_slice->piece_from[pos->turn()][moving     ][from_square];

    // undefended_bbs - squares which are not attacked by COLOR 

    pos->update_attacks(WHITE);
    pos->update_attacks(BLACK);

    undefended_bbs[WHITE] = ~pos->attacked_by(WHITE);
    undefended_bbs[BLACK] = ~pos->attacked_by(BLACK);

    // unsafe_bbs - squares for PIECE of turn which are not safe for them, ie they would either hang or allow a piece worth less to capture them

    const BB hanging_squares = pos->attacked_by(pos->notturn()) & ~pos->attacked_by(pos->turn());
    BB cum_attacks = BB_EMPTY;

    unsafe_bbs[PAWN] = hanging_squares;
    cum_attacks |= pos->attacked_by(pos->notturn(), PAWN);

    unsafe_bbs[KNIGHT] = hanging_squares | cum_attacks;
    unsafe_bbs[BISHOP] = hanging_squares | cum_attacks;
    cum_attacks |= pos->attacked_by(pos->notturn(), KNIGHT);
    cum_attacks |= pos->attacked_by(pos->notturn(), BISHOP);

    unsafe_bbs[ROOK] = hanging_squares | cum_attacks;
    cum_attacks |= pos->attacked_by(pos->notturn(), ROOK);
    
    unsafe_bbs[QUEEN] = hanging_squares | cum_attacks;

    unsafe_bbs[KING] = pos->attacked_by(pos->notturn());

    // tempo_bbs - squares which would yeild "tempo" if a PIECE of turn were to move there, including checks

    const BB occupied = pos->pieces() & ~pos->pieces(pos->notturn(), KING);

    BB cum_victims = undefended_bbs[pos->notturn()] & pos->pieces(pos->notturn());

    tempo_bbs[KING] = attacks::kings(cum_victims);
    cum_victims |= pos->pieces(pos->notturn(), KING);

    tempo_bbs[QUEEN] = attacks::queens(cum_victims, occupied);
    cum_victims |= pos->pieces(pos->notturn(), QUEEN);

    tempo_bbs[ROOK] = attacks::rooks(cum_victims, occupied);
    cum_victims |= pos->pieces(pos->notturn(), ROOK);

    tempo_bbs[BISHOP] = attacks::bishops(cum_victims, occupied);
    tempo_bbs[KNIGHT] = attacks::knights(cum_victims);
    cum_victims |= pos->pieces(pos->notturn(), BISHOP);
    cum_victims |= pos->pieces(pos->notturn(), KNIGHT);
    
    tempo_bbs[PAWN] = attacks::pawns(cum_victims, pos->notturn());

    // check_bbs

    enemy_king_square = bb_peek(pos->pieces(pos->notturn(), KING));

    check_bbs[PAWN]   = attacks::pawn  (enemy_king_square, pos->notturn());
    check_bbs[KNIGHT] = attacks::knight(enemy_king_square                );
    check_bbs[BISHOP] = attacks::bishop(enemy_king_square, occupied      );
    check_bbs[ROOK]   = attacks::rook  (enemy_king_square, occupied      );
    check_bbs[QUEEN]  = attacks::queen (enemy_king_square, occupied      );
    check_bbs[KING]   = BB_EMPTY;

}

template<MovePickerStage STAGE>
Score MovePicker::score_move(const Move move) {
    
    constexpr Score capture_values  [N_PIECES + 1] = { 100, 300, 300, 500, 900, 0, 0 };
    constexpr Score promotion_values[N_PIECES    ] = { 0, 100, -10000, -10000, 60, 0 };

    if constexpr (STAGE == STAGE_TT_COUNTERS)
        return SCORE_MIN + 5 * (move == tt_move) + 4 * (move == from_to_counter) + 3 * (move == piece_from_counter) + 2 * (move == piece_to_counter);

    const Square from_square   = move::from_square   (move);
    const Square to_square     = move::to_square     (move);
    const Piece  moving        = pos->piece_on(from_square);
    const BB     from_bb       = bb_of(from_square);
    const BB     to_bb         = bb_of(to_square);

    if constexpr (STAGE == STAGE_LOUDS) {

        const Square victim_square = move::capture_square(move);
        const Piece  victim        = pos->piece_on(victim_square);
        
        const BB   post_occupied     = (pos->pieces() & ~(from_bb | bb_of(victim_square))) | to_bb;
        const bool is_revealed_check = (attacks::bishop(enemy_king_square, post_occupied) & pos->pieces(pos->turn(), BISHOP))
                                    | (attacks::rook  (enemy_king_square, post_occupied) & pos->pieces(pos->turn(), ROOK  ));

        const bool is_check     = (check_bbs[moving] & to_bb) || is_revealed_check;
        const bool is_capture   = move::is_capture(move);
        const bool is_promotion = move::is_promotion(move);
        const bool is_saving    = (unsafe_bbs[moving] & from_bb) && !(unsafe_bbs[moving] & to_bb);

        if (STAGE == STAGE_LOUDS && !(is_check || is_capture || is_promotion || is_saving))
            return SCORE_MIN;

        const bool is_tempo = tempo_bbs[moving] & to_bb;

        const Score material_value = capture_values[victim]
                                   + capture_values[moving] * ((unsafe_bbs[moving] & from_bb) - (unsafe_bbs[moving] & to_bb))
                                   + promotion_values[move::promotion_piece(move)] * is_promotion;

        return SCORE_ZERO + 10000 * is_check + 50 * is_tempo + material_value;
    }

    if constexpr (STAGE == STAGE_QUIETS) {

        const bool is_tempo = tempo_bbs[moving] & to_bb;

        const Score material_value = capture_values[moving] * ((unsafe_bbs[moving] & from_bb) - (unsafe_bbs[moving] & to_bb));

        return SCORE_ZERO + 50 * is_tempo + material_value;

    }

    assert(false);
}

template<MovePickerStage STAGE>
void MovePicker::score_moves() {

    num_left_in_stage = 0;
    
    for (size_t i = 0; i < num_left; i++) {
        
        scores[i] = score_move<STAGE>(moves[i]);

        if (scores[i] != SCORE_MIN) {

            Move  temp_move  = moves [num_left_in_stage];
            Score temp_score = scores[num_left_in_stage];
            
            moves [num_left_in_stage] = moves [i];
            scores[num_left_in_stage] = scores[i];

            moves [i] = temp_move;
            scores[i] = temp_score;

            num_left_in_stage++;

        }
    }

}

Move MovePicker::pop() {

    if (num_left == 0)
        return MOVE_NONE;

    while (num_left_in_stage == 0)
        next_stage();

    assert(num_left);
    assert(num_left_in_stage);

    size_t best_index = 0;
    Score  best_score = scores[0];

    for (size_t i = 1; i < num_left_in_stage; i++) {
        if (scores[i] > best_score) {
            best_score = scores[i];
            best_index = i;
        }
    }

    num_left--;
    num_left_in_stage--;

    Move best_move = moves[best_index];

    moves [best_index] = moves [num_left_in_stage];
    scores[best_index] = scores[num_left_in_stage];

    moves [num_left_in_stage] = moves[num_left];

    // std::cout << std::to_string(best_score) << std::endl;

    return best_move;

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
