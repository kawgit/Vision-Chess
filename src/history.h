#include "pos.h"
#include "types.h"

struct HistorySlice {
    static const size_t N_COUNTERS = 3;

    Score piece_to  [N_COLORS][N_PIECES ][N_SQUARES];
    Score from_to   [N_COLORS][N_SQUARES][N_SQUARES];
    Score piece_from[N_COLORS][N_PIECES ][N_SQUARES];

    // Move counter_piece_to  [N_COLORS][N_PIECES ][N_SQUARES][N_COUNTERS];
    // Move counter_from_to   [N_COLORS][N_SQUARES][N_SQUARES][N_COUNTERS];
    // Move counter_piece_from[N_COLORS][N_PIECES ][N_SQUARES][N_COUNTERS];

    void update_bonus  (const Square from_square, const Square to_square, const Piece piece, const Color color, const Score bonus);
    // void update_counter(const Square from_square, const Square to_square, const Piece piece, const Color color, const Move counter);
};

struct History {

    HistorySlice  history_slice_stack[INTERNAL_PLY_LIMIT + 6 + 2];
    HistorySlice* history_slice;

    History();

    void update_bonus  (const Move move, const Pos& pos, const Score bonus);
    // void update_counter(const Move move, const Pos& pos, const Move counter);

    void push();

    void pop();

    Score score_move(const Square from_square, const Square to_square, const Piece piece, const Color color) const;

};