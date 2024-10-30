#pragma once

#include <immintrin.h>

#include "accumulator.h"
#include "pos.h"
#include "types.h"

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
    const size_t simd_size = 256 / (8 * sizeof(float));

    const size_t IN_LEN = N_COLORS * N_PIECES * N_SQUARES;
    const size_t L1_LEN = 1024;
    const size_t L2_LEN = 16;
    const size_t L3_LEN = 16;
    const size_t OP_LEN = 1;

    extern float accumulator[N_COLORS][L1_LEN];

    extern float in_l1_weights[IN_LEN * L1_LEN];
    extern float l1_l2_weights[L1_LEN * L2_LEN];
    extern float l2_l3_weights[L2_LEN * L3_LEN];
    extern float l3_op_weights[L3_LEN * OP_LEN];

    extern float l1_biases[L1_LEN];
    extern float l2_biases[L2_LEN];
    extern float l3_biases[L3_LEN];
    extern float op_biases[OP_LEN];

    void read_network(std::string path);

	void add_piece(const Color color, const Piece piece, const Square square);

	void rem_piece(const Color color, const Piece piece, const Square square);

    template <size_t SRC_LEN, size_t DST_LEN>
    void forward_pass(const float* src, float* dst, const float* weights, const float* biases);
*/

    Eval evaluate(Accumulator& accumulator, Pos& pos);
}