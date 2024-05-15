#include <algorithm>
#include <immintrin.h>
#include <cstring>
#include <fstream>

#include "accumulator.h"
#include "nnue.h"
#include "pos.h"
#include "types.h"

using namespace nnue;

AccumulatorSlice::AccumulatorSlice() {
    
    std::memcpy(values, l1_biases, sizeof(l1_biases));
    std::memset(accurate, 0, sizeof(accurate));
    std::memset(flipped,  0, sizeof(flipped));
    std::memset(vcp_bbs,  0, sizeof(vcp_bbs));

    std::cout << "helo?" << std::endl;

}

void AccumulatorSlice::add_piece(const Color view, const Color color, const Piece piece, const Square square) {

    assert(!(vcp_bbs[view][color][piece] & bb_of(square)));

    vcp_bbs[view][color][piece] |= bb_of(square);

}

void AccumulatorSlice::rem_piece(const Color view, const Color color, const Piece piece, const Square square) {

    assert(vcp_bbs[view][color][piece] & bb_of(square));

    vcp_bbs[view][color][piece] &= ~bb_of(square);

}

Accumulator::Accumulator() {

    slice = slice_stack;

}

const AccumulatorSlice* Accumulator::find_best_parent(const Color view, const Pos& pos) {

    // Number of operations (sets/unsets) needed in best scenario
    int best_ops = 0;
    const AccumulatorSlice* best_parent = nullptr;

    // Check ops if we rebuild values from scratch
    for (Color color : { BLACK, WHITE })
        for (Piece piece = PAWN; piece <= KING; piece++)
            best_ops += bitcount(pos.pieces(color, piece));
    best_ops -= 1; // since a+b+c has 3 - 1 = 2 operations

    // Check ops if we use a parent
    for (const AccumulatorSlice* curr = slice; slice >= slice_stack; slice--) {
        
        int curr_ops = 0;

        for (Color color : { BLACK, WHITE })
            for (Piece piece = PAWN; piece <= KING; piece++)
                curr_ops += bitcount(pos.pieces(color, piece) ^ curr->vcp_bbs[view][color][piece]);

        if (curr_ops < best_ops) {
            best_ops = curr_ops;
            best_parent = curr;
        }
    } 

    return best_parent;

};

void Accumulator::update_view(const Color view, const Pos& pos) {

    const AccumulatorSlice* parent = find_best_parent(view, pos);

    if (parent == nullptr) {
        // Rebuild values from scratch
    }
    else {
        // Build values by adding or removing features from a parent
    }

};

void Accumulator::update(const Pos& pos) {

    update_view(WHITE, pos);
    update_view(BLACK, pos);

};

// void Accumulator::reset(const Pos& pos) {

//     slice = slice_stack;
//     slice->reset(pos);

// }

// void Accumulator::recursively_update(const Pos& pos, size_t offset) {

//     AccumulatorSlice* acc_slice =     slice - offset;
//     const Slice*      pos_slice = pos.slice - offset;

//     if (acc_slice->accurate)
//         return;

//     assert(acc_slice !=     slice_stack);
//     assert(pos_slice != pos.slice_stack);
    
//     if (!(acc_slice - 1)->accurate)
//         recursively_update(pos, offset + 1);
    
// 	*acc_slice = *(acc_slice - 1);

//     const Move  move   = pos_slice->move;
//     const Piece moving = pos_slice->moving;
//     const Piece victim = pos_slice->victim;
//     const Piece placed = move::is_promotion(move) ? move::promotion_piece(move) : moving;
    
//     const Square from_square = move::from_square(move);
// 	const Square to_square   = move::to_square(move);

//     const Color turn = offset % 2 ? pos.turn() : pos.notturn();
//     const Color notturn = !turn;
	
// 	acc_slice->rem_piece(turn, moving, from_square);

// 	if (move::is_capture(move)) {
// 		Square victim_square = move::capture_square(move);

// 		acc_slice->rem_piece(notturn, pos_slice->victim, victim_square);
// 	}

// 	acc_slice->add_piece(turn, placed, to_square);

// 	if (move::is_king_castle(move)) {
// 		Square offset = (turn == BLACK) * (N_FILES * 7);
// 		Square rook_from = H1 + offset;
// 		Square rook_to   = F1 + offset;

// 		acc_slice->rem_piece(turn, ROOK, rook_from);
// 		acc_slice->add_piece(turn, ROOK, rook_to);
// 	}
// 	else if (move::is_queen_castle(move)) {
// 		Square offset = (turn == BLACK) * (N_FILES * 7);
// 		Square rook_from = A1 + offset;
// 		Square rook_to   = D1 + offset;

// 		acc_slice->rem_piece(turn, ROOK, rook_from);
// 		acc_slice->add_piece(turn, ROOK, rook_to);
// 	}

//     acc_slice->accurate = true;

// }

// void Accumulator::push() {
//     slice++;
//     slice->accurate = false;
// }

// void Accumulator::pop() {
//     slice--;
// }