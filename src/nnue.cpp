#include <algorithm>
#include <immintrin.h>
#include <cstring>
#include <fstream>

#include "nnue.h"
#include "pos.h"
#include "types.h"
#include "accumulator.h"

namespace nnue {

ALIGN64 Eval fkps_weights[N_FBUCKETS][N_KBUCKETS][N_COLORS][N_PIECES][N_SQUARES];

// ALIGN64 float in_l1_weights[IN_LEN * L1_LEN];
// ALIGN64 float l1_l2_weights[L1_LEN * L2_LEN];
// ALIGN64 float l2_l3_weights[L2_LEN * L3_LEN];
// ALIGN64 float l3_op_weights[L3_LEN * OP_LEN];

// ALIGN64 float l1_biases[L1_LEN];
// ALIGN64 float l2_biases[L2_LEN];
// ALIGN64 float l3_biases[L3_LEN];
// ALIGN64 float op_biases[OP_LEN];

size_t get_file_size(std::ifstream& file) {
    file.seekg(0,std::ios_base::end);
    size_t file_size = file.tellg();
    file.seekg(0,std::ios_base::beg);
    return file_size;
}

template <typename T>
static size_t read_contents(const char* data, T* output, size_t count) {

    for (size_t i = 0; i < count; i++)
        output[i] = *((T*)(data + sizeof(T) * i));

    return sizeof(T) * count;
}

void init(std::string path) {

    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        std::cout << "Could not open network file" << std::endl;
        return;
    }

    const size_t file_size = get_file_size(file);
    char* buff = new char[file_size];
    file.read(buff, file_size);
    
    char* curr = buff;

    curr += read_contents<Eval>(curr, (Eval*) fkps_weights, N_FBUCKETS * N_KBUCKETS * N_COLORS * N_PIECES * N_SQUARES);

    assert((curr - buff) == file_size);

    delete [] buff;
}

// void read_network(std::string path) {

//     std::ifstream file(path, std::ios::binary);

//     if (!file.is_open()) {
//         std::cout << "Could not open network file" << std::endl;
//         return;
//     }

//     const size_t file_size = get_file_size(file);
//     char* buff = new char[file_size];
//     file.read(buff, file_size);

//     char* curr = buff;

//     curr += read_contents<float>(curr, in_l1_weights, IN_LEN * L1_LEN);
//     curr += read_contents<float>(curr, l1_biases,     L1_LEN);

//     curr += read_contents<float>(curr, l1_l2_weights, L1_LEN * L2_LEN);
//     curr += read_contents<float>(curr, l2_biases,     L2_LEN);

//     curr += read_contents<float>(curr, l2_l3_weights, L2_LEN * L3_LEN);
//     curr += read_contents<float>(curr, l3_biases,     L3_LEN);

//     curr += read_contents<float>(curr, l3_op_weights, L3_LEN * OP_LEN);
//     curr += read_contents<float>(curr, op_biases,     OP_LEN);

//     assert((curr - buff) == file_size);

//     delete [] buff;

// 	std::memcpy(accumulator, l1_biases, sizeof(accumulator[0]));

// }

// inline size_t feature_index(const bool ours, const Piece piece, const Square square) {
//     return ours * N_PIECES * N_SQUARES + piece * N_SQUARES + square;
// }

// void add_feature(const Color color, const size_t index) {
//     float* src = in_l1_weights + index * L1_LEN;
//     float* dst = accumulator[color];
    
//     for (size_t i = 0; i < L1_LEN / simd_size; i++) {
//         __m256 vsrc = _mm256_load_ps(src + i * simd_size);
//         __m256 vdst = _mm256_load_ps(dst + i * simd_size);
    
//         vdst = _mm256_add_ps(vdst, vsrc);

//         _mm256_store_ps(dst + i * simd_size, vdst);
//     }
// }

// void rem_feature(const Color color, const size_t index) {
//     float* src = in_l1_weights + index * L1_LEN;
//     float* dst = accumulator[color];
    
//     for (size_t i = 0; i < L1_LEN / simd_size; i++) {
//         __m256 vsrc = _mm256_load_ps(src + i * simd_size);
//         __m256 vdst = _mm256_load_ps(dst + i * simd_size);
    
//         vdst = _mm256_sub_ps(vdst, vsrc);

//         _mm256_store_ps(dst + i * simd_size, vdst);
//     }
// }

// template <size_t SRC_LEN, size_t DST_LEN>
// void forward_pass(const float* src, float* dst, const float* weights, const float* biases) {

//     if constexpr (DST_LEN == 1) {

//         assert(SRC_LEN % simd_size == 0);

//         __m256 vacc = _mm256_set1_ps(0);

//         for (size_t i = 0; i < SRC_LEN / simd_size; i++) {
            
//             __m256 vsrc = _mm256_load_ps(src + i * simd_size);
//             __m256 vwgt = _mm256_load_ps(weights + i * simd_size);

//             vacc = _mm256_fmadd_ps(vsrc, vwgt, vacc); 

//         }

//         _mm256_store_ps(dst, vacc);

//         for (size_t i = 1; i < simd_size; i++) {
//             dst[0] += dst[i];
//         }

//     }
//     else {
        
//         assert(SRC_LEN % simd_size == 0);
//         assert(DST_LEN % simd_size == 0);

//         for (size_t o = 0; o < DST_LEN / simd_size; o++) {
        
//             __m256 vacc = _mm256_load_ps(biases + o * simd_size);
        
//             for (size_t i = 0; i < SRC_LEN / simd_size; i++) {
                
//                 __m256 vsrc = _mm256_load_ps(src + i * simd_size);
//                 __m256 vwgt = _mm256_load_ps(weights + i * simd_size * DST_LEN + o * simd_size);

//                 vacc = _mm256_fmadd_ps(vsrc, vwgt, vacc); 

//             }

//             _mm256_store_ps(dst + o * simd_size, vacc);
//         }

//     }

// }

// float evaluate(Color color) {

//     constexpr size_t MAX_LEN = L1_LEN;
    
//     ALIGN64 float floats_0[MAX_LEN];
//     ALIGN64 float floats_1[MAX_LEN];

//     forward_pass<L1_LEN, L2_LEN>(accumulator[color], floats_0, l1_l2_weights, l2_biases);
//     forward_pass<L2_LEN, L3_LEN>(floats_0,           floats_1, l2_l3_weights, l3_biases);
//     forward_pass<L3_LEN, OP_LEN>(floats_1,           floats_0, l3_op_weights, op_biases);

//     return floats_0[0];
// }

Eval evaluate(Accumulator& accumulator, Pos& pos) {
    accumulator.recursively_update(pos);
    return (accumulator.slice->values[pos.turn()] - accumulator.slice->values[pos.notturn()]) / 2;
}

}