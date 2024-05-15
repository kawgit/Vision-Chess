#include <cassert>

#include "history.h"
#include "move.h"

History::History() {
    history_slice = history_slice_stack + 6;
}

void History::update_bonus(const Move move, const Pos& pos, const Score bonus) {
    const Square from_square = move::from_square(move);
    const Square to_square   = move::to_square(move);
    const Piece  piece       = pos.piece_on(from_square);
    const Color  color       = pos.turn();

    for (int i = -6; i <= 2; i += 2) {
        
        HistorySlice* curr = history_slice + i;
        
        assert(curr != history_slice_stack - 1);
        assert(curr != history_slice_stack + 256 + 6 + 2);

        curr->update_bonus(from_square, to_square, piece, color, bonus);
    }
}

void HistorySlice::update_bonus(const Square from_square, const Square to_square, const Piece piece, const Color color, const Score bonus) {
    piece_to  [color][piece      ][to_square  ] += bonus;
    piece_from[color][piece      ][from_square] += bonus;
    from_to   [color][from_square][to_square  ] += bonus;
}

void History::push() {
    history_slice++;
}

void History::pop() {
    history_slice--;
}

Score History::score_move(const Square from_square, const Square to_square, const Piece piece, const Color color) const {
    return history_slice->piece_to  [color][piece      ][to_square  ]
         + history_slice->piece_from[color][piece      ][from_square]
         + history_slice->from_to   [color][from_square][to_square  ];
}
