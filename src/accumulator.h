#pragma once

#include "nnue.h"
#include "pos.h"
#include "types.h"

namespace nnue {

struct AccumulatorSlice {

    bool accurate[N_COLORS] = { false };
    bool flipped[N_COLORS];
    int16_t values[N_COLORS][L1_LEN / 2];

    BB vcp_bbs[N_COLORS][N_COLORS][N_PIECES];

    void add_piece(const Color view, const Color color, const Piece piece, const Square square);
    void rem_piece(const Color view, const Color color, const Piece piece, const Square square);

    void add_feature(const Color view, const size_t idx);
    void rem_feature(const Color view, const size_t idx);    

    void reset(const Pos& pos);

};

struct Accumulator {

    AccumulatorSlice  slice_stack[INTERNAL_PLY_LIMIT];
    AccumulatorSlice* slice;

    const AccumulatorSlice* find_best_parent(const Color view, const Pos& pos);
    void update_view(const Color view, const Pos& pos);
    void update(const Pos& pos);

    // void reset(const Pos& pos);

    // void push();
    // void pop();

    // void update_static(const Pos& pos);
    // void update_recursively(const Pos& pos, size_t offset = 0);

};

}