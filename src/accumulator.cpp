#include <algorithm>
#include <immintrin.h>
#include <cstring>
#include <fstream>

#include "accumulator.h"
#include "nnue.h"
#include "pos.h"
#include "types.h"

using namespace nnue;

// constexpr uint8_t phase_weights[N_PIECES + 1] = {0, 3, 4, 8, 18, 0, 0};
// constexpr uint8_t phase_start = (phase_weights[0] * 16 
//                                + phase_weights[1] * 4 
//                                + phase_weights[2] * 4 
//                                + phase_weights[3] * 4 
//                                + phase_weights[4] * 2);

// inline size_t calc_fbucket(const uint8_t phase) {

//     return std::min(phase * N_FBUCKETS / phase, N_FBUCKETS - 1);
// }

// inline size_t calc_kbucket(const Color view, const Square king_square) {
    
//     const Square square_adj = flip_components(king_square, view == BLACK, file_of(king_square) >= FILE_E);

//     assert(file_of(square_adj) <= FILE_D);

//     return rank_of(square_adj) * 4 + file_of(square_adj);
// }

void AccumulatorSlice::add_piece(const Color view, const Color color, const Piece piece, const Square square) {

    assert(!(vcp_bbs[view][color][piece] & bb_of(square)));

    vcp_bbs[view][color][piece] |= bb_of(square);

}

void AccumulatorSlice::rem_piece(const Color view, const Color color, const Piece piece, const Square square) {

    assert(vcp_bbs[view][color][piece] & bb_of(square));

    vcp_bbs[view][color][piece] &= ~bb_of(square);

}

// void AccumulatorSlice::reset(const Pos& pos) {

//     accurate = true;
//     fbucket = calc_fbucket(phase);

// 	std::memset(color_piece_bbs, 0, sizeof(color_piece_bbs));

//     BB occupied = pos.pieces();

//     while (occupied) {
//         Square square = poplsb(occupied);
//         Piece piece = pos.piece_on(square);
//         Color color = pos.color_on(square);

//         add_piece(color, piece, square);
//     }

//     rebucket(WHITE);
//     rebucket(BLACK);
// }

// void AccumulatorSlice::reset_values(const Color view) {
//     values[view] = 0;
// }

// void AccumulatorSlice::rebucket(const Color view) {

//     reset_values(view);
//     update_buckets(view);

//     for (Color color : {WHITE, BLACK}) {
//         for (Piece piece = PAWN; piece <= KING; piece++) {

//             BB squares = color_piece_bbs[color][piece];

//             while (squares) {

//                 const Square square = flip_components(poplsb(squares), view == BLACK, flipped[view]);

//                 values[view] += fkps_weights[fbucket][kbucket[view]][color == view][piece][square];

//             }

//         }
//     }

// }

// void AccumulatorSlice::update_buckets(const Color view) {

//     assert(color_piece_bbs[view][KING]);
//     assert(!bb_has_multiple(color_piece_bbs[view][KING]));

//     Square king_square = lsb(color_piece_bbs[view][KING]);
//     flipped[view]      = file_of(king_square) >= FILE_E;
//     kbucket[view]      = calc_kbucket(view, king_square);

//     fbucket = calc_fbucket(phase);

//     assert(kbucket[view] < N_KBUCKETS);
//     assert(fbucket       < N_FBUCKETS);

// }

template<bool S1, bool S2, bool U1, bool U2>
void apply_change(AccumulatorSlice* src, AccumulatorSlice* dst, const Color view, const int s1, const int s2, const int u1, const int u2) {

    const __m256i* vsrc = (__m256i*) src->values[view];
    const __m256i* vws1 = (__m256i*) l1_weights[s1];
    const __m256i* vws2 = (__m256i*) l1_weights[s2];
    const __m256i* vwu1 = (__m256i*) l1_weights[u1];
    const __m256i* vwu2 = (__m256i*) l1_weights[u2];
          __m256i* vout = (__m256i*) dst->values[view];

    constexpr size_t UNROLL = N_REGISTERS / 2;

    __m256i registers[UNROLL];

    static_assert(L1_LEN / 2 / SIMD_WIDTH_EPI16 % UNROLL == 0);

    for (size_t i = 0; i < L1_LEN / 2 / SIMD_WIDTH_EPI16; i += UNROLL) {
        
        for (size_t j = 0; j < UNROLL; j++) {
            
            registers[j] = vsrc[i + j];

            if constexpr (S1)
                registers[j] = _mm256_add_epi16(registers[j], vws1[i + j]);

            if constexpr (S2)
                registers[j] = _mm256_add_epi16(registers[j], vws2[i + j]);
                
            if constexpr (U1)
                registers[j] = _mm256_sub_epi16(registers[j], vwu1[i + j]);
                
            if constexpr (U2)
                registers[j] = _mm256_sub_epi16(registers[j], vwu2[i + j]);
                
            vout[i + j] = registers[j];

        }
    }
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
        // Incrementally update from parent
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