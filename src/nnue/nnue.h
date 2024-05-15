#pragma once

#include <immintrin.h>

#include "types.h"
#include "pos.h"
#include "accumulator.h"

#define ALIGN64 alignas(64)

namespace nnue {

    constexpr size_t L0_LEN = N_COLORS * N_PIECES * N_SQUARES;
    constexpr size_t L1_LEN = 1024;
    constexpr size_t L2_LEN = 32;
    constexpr size_t L3_LEN = 16;
    constexpr size_t L4_LEN = 1;

    // presuming AVX-256 build
    constexpr size_t N_REGISTERS = 16;
    constexpr size_t SIMD_WIDTH_EPI32 = 8;
    constexpr size_t SIMD_WIDTH_EPI16 = 16;
    constexpr size_t SIMD_WIDTH_EPI8  = 32;

    extern int16_t l1_weights[L0_LEN][L1_LEN];
    extern int16_t l1_biases [L1_LEN];
    extern int8_t  l2_weights[L1_LEN][L2_LEN];
    extern int16_t l2_biases [L2_LEN];
    extern int8_t  l3_weights[L2_LEN][L3_LEN];
    extern int16_t l3_biases [L3_LEN];
    extern int8_t  l3_weights[L3_LEN][L4_LEN];
    extern int16_t l3_biases [L4_LEN];

    void init(std::string path);

/*
    void read_network(std::string path);

	void add_piece(const Color color, const Piece piece, const Square square);

	void rem_piece(const Color color, const Piece piece, const Square square);

    template <size_t SRC_LEN, size_t DST_LEN>
    void forward_pass(const float* src, float* dst, const float* weights, const float* biases);
*/

    Eval evaluate(Accumulator& accumulator, Pos& pos);
}

struct NNUE : Evaluator {

    Accumulator accum;

    void reset(const Pos& pos);
    void push();
    void pop();
    Eval evaluate(const Pos& pos);

}