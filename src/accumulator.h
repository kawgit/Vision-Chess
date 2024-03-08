#pragma once

#include "pos.h"
#include "types.h"


struct AccumulatorSlice {

    bool accurate = false;
    uint8_t phase;

    BB                  color_piece_bbs[N_COLORS][N_PIECES];

    size_t              fbucket;
    size_t              kbucket[N_COLORS];
    bool                flipped[N_COLORS];
    Eval                values [N_COLORS];

    void add_piece(const Color color, const Piece piece, const Square square);

    void rem_piece(const Color color, const Piece piece, const Square square);

    void add_values(const Color color, const Piece piece, const Square square);

    void rem_values(const Color color, const Piece piece, const Square square);

    void reset(const Pos& pos);
    
    void reset_values(const Color view);

    void rebucket(const Color view);
    
    void update_buckets(const Color view);

};

struct Accumulator {

    AccumulatorSlice slice_stack[256];
    AccumulatorSlice* slice;

    void reset(const Pos& pos);

    void push();
    
    void pop();

    void recursively_update(const Pos& pos, size_t offset = 0);

};