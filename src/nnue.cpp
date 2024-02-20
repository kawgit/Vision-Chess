#include <algorithm>
#include <immintrin.h>
#include <cstring>
#include <fstream>

#include "nnue.h"
#include "pos.h"
#include "types.h"

namespace nnue {

Eval ps_weights[N_PIECES][N_SQUARES] = {
        { 
            100,  100,  100,  100,  100,  100,  100,  100,   89,
            91,   96,  114,  141,  164,  173,  159,   98,  105,
            98,  101,  116,  124,  128,  116,   89,  101,   90,
            96,   96,   92,   99,   91,   91,  105,   98,   99,
            91,   86,   94,   86,  118,  136,  127,  118,   99,
            95,   95,   91,  144,  162,  163,  142,  114,  112,
            119,  116,  100,  100,  100,  100,  100,  100,  100,
            100
        },
        { 201,  260,  270,  287,  288,  283,  269,  222,  276,
            285,  302,  305,  300,  298,  285,  271,  291,  314,
            325,  328,  319,  308,  304,  278,  302,  320,  331,
            319,  323,  313,  301,  291,  309,  318,  328,  322,
            321,  315,  304,  287,  296,  317,  321,  327,  320,
            312,  300,  270,  283,  299,  307,  309,  299,  294,
            277,  265,  218,  272,  283,  292,  283,  275,  259,
            202
        },
        { 
            289,  305,  309,  312,  319,  306,  323,  316,  315,
            325,  325,  325,  320,  329,  321,  314,  333,  337,
            341,  335,  337,  327,  331,  311,  319,  331,  335,
            342,  339,  333,  326,  316,  322,  328,  339,  341,
            340,  329,  327,  313,  325,  337,  336,  343,  331,
            332,  329,  322,  322,  322,  324,  321,  324,  328,
            325,  310,  304,  314,  302,  317,  315,  312,  319,
            307
        },
        { 
            474,  486,  495,  499,  497,  498,  498,  491,  483,
            492,  502,  503,  504,  503,  497,  492,  489,  505,
            508,  505,  501,  498,  498,  497,  494,  501,  504,
            499,  505,  502,  500,  497,  493,  504,  508,  503,
            502,  500,  500,  496,  494,  508,  507,  505,  501,
            498,  496,  495,  489,  498,  503,  505,  503,  502,
            492,  491,  481,  495,  500,  500,  497,  495,  491,
            485
        },
        { 
            920,  918,  929,  927,  939,  934,  931,  921,  931,
            945,  948,  945,  940,  941,  930,  924,  955,  958,
            969,  950,  943,  931,  933,  922,  941,  946,  951,
            948,  942,  935,  927,  928,  956,  942,  947,  948,
            942,  937,  927,  919,  951,  960,  955,  951,  943,
            940,  930,  922,  953,  948,  948,  945,  941,  933,
            913,  907,  927,  926,  931,  943,  934,  931,  928,
            918
        },
        { 
            -23,   83,   57,   42,   39,   93,   98,   14,   49,
            94,   91,   80,   76,   99,  121,   85,   55,  117,
            110,   82,   82,   92,  109,   75,   25,   63,   54,
            65,   45,   63,   72,   39,  -17,   10,   23,   25,
            16,   18,   23,   -9,  -29,   -7,   -0,    3,  -19,
            -20,  -20,  -41,  -26,  -13,  -18,  -27,  -52,  -51,
            -48,  -55,  -27,  -26,  -51,  -64, -102,  -79,  -50,
            -72
        },
    };


/*
inline size_t feature_index(const bool ours, const Piece piece, const Square square) {
    return ours * N_PIECES * N_SQUARES + piece * N_SQUARES + square;
}

ALIGN64 float in_l1_weights[IN_LEN * L1_LEN];
ALIGN64 float l1_l2_weights[L1_LEN * L2_LEN];
ALIGN64 float l2_l3_weights[L2_LEN * L3_LEN];
ALIGN64 float l3_op_weights[L3_LEN * OP_LEN];

ALIGN64 float l1_biases[L1_LEN];
ALIGN64 float l2_biases[L2_LEN];
ALIGN64 float l3_biases[L3_LEN];
ALIGN64 float op_biases[OP_LEN];

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

void read_network(std::string path) {

    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        std::cout << "Could not open network file" << std::endl;
        return;
    }

    const size_t file_size = get_file_size(file);
    char* buff = new char[file_size];
    file.read(buff, file_size);

    char* curr = buff;

    curr += read_contents<float>(curr, in_l1_weights, IN_LEN * L1_LEN);
    curr += read_contents<float>(curr, l1_biases,     L1_LEN);

    curr += read_contents<float>(curr, l1_l2_weights, L1_LEN * L2_LEN);
    curr += read_contents<float>(curr, l2_biases,     L2_LEN);

    curr += read_contents<float>(curr, l2_l3_weights, L2_LEN * L3_LEN);
    curr += read_contents<float>(curr, l3_biases,     L3_LEN);

    curr += read_contents<float>(curr, l3_op_weights, L3_LEN * OP_LEN);
    curr += read_contents<float>(curr, op_biases,     OP_LEN);

    assert((curr - buff) == file_size);

    delete [] buff;

	std::memcpy(accumulator, l1_biases, sizeof(accumulator[0]));

}

inline size_t feature_index(const bool ours, const Piece piece, const Square square) {
    return ours * N_PIECES * N_SQUARES + piece * N_SQUARES + square;
}

void add_feature(const Color color, const size_t index) {
    float* src = in_l1_weights + index * L1_LEN;
    float* dst = accumulator[color];
    
    for (size_t i = 0; i < L1_LEN / simd_size; i++) {
        __m256 vsrc = _mm256_load_ps(src + i * simd_size);
        __m256 vdst = _mm256_load_ps(dst + i * simd_size);
    
        vdst = _mm256_add_ps(vdst, vsrc);

        _mm256_store_ps(dst + i * simd_size, vdst);
    }
}

void rem_feature(const Color color, const size_t index) {
    float* src = in_l1_weights + index * L1_LEN;
    float* dst = accumulator[color];
    
    for (size_t i = 0; i < L1_LEN / simd_size; i++) {
        __m256 vsrc = _mm256_load_ps(src + i * simd_size);
        __m256 vdst = _mm256_load_ps(dst + i * simd_size);
    
        vdst = _mm256_sub_ps(vdst, vsrc);

        _mm256_store_ps(dst + i * simd_size, vdst);
    }
}

void add_piece(const Color color, const Piece piece, const Square square) {
    for (const Color side : {WHITE, BLACK}) {
        const bool ours = color == side;
        const size_t index = feature_index(ours, piece, square);
        add_feature(side, index);
    }
}

void rem_piece(const Color color, const Piece piece, const Square square) {
    for (const Color side : {WHITE, BLACK}) {
        const bool ours = color == side;
        const size_t index = feature_index(ours, piece, square);
        rem_feature(side, index);
    }
}

template <size_t SRC_LEN, size_t DST_LEN>
void forward_pass(const float* src, float* dst, const float* weights, const float* biases) {

    if constexpr (DST_LEN == 1) {

        assert(SRC_LEN % simd_size == 0);

        __m256 vacc = _mm256_set1_ps(0);

        for (size_t i = 0; i < SRC_LEN / simd_size; i++) {
            
            __m256 vsrc = _mm256_load_ps(src + i * simd_size);
            __m256 vwgt = _mm256_load_ps(weights + i * simd_size);

            vacc = _mm256_fmadd_ps(vsrc, vwgt, vacc); 

        }

        _mm256_store_ps(dst, vacc);

        for (size_t i = 1; i < simd_size; i++) {
            dst[0] += dst[i];
        }

    }
    else {
        
        assert(SRC_LEN % simd_size == 0);
        assert(DST_LEN % simd_size == 0);

        for (size_t o = 0; o < DST_LEN / simd_size; o++) {
        
            __m256 vacc = _mm256_load_ps(biases + o * simd_size);
        
            for (size_t i = 0; i < SRC_LEN / simd_size; i++) {
                
                __m256 vsrc = _mm256_load_ps(src + i * simd_size);
                __m256 vwgt = _mm256_load_ps(weights + i * simd_size * DST_LEN + o * simd_size);

                vacc = _mm256_fmadd_ps(vsrc, vwgt, vacc); 

            }

            _mm256_store_ps(dst + o * simd_size, vacc);
        }

    }

}

float evaluate(Color color) {

    constexpr size_t MAX_LEN = L1_LEN;
    
    ALIGN64 float floats_0[MAX_LEN];
    ALIGN64 float floats_1[MAX_LEN];

    forward_pass<L1_LEN, L2_LEN>(accumulator[color], floats_0, l1_l2_weights, l2_biases);
    forward_pass<L2_LEN, L3_LEN>(floats_0,           floats_1, l2_l3_weights, l3_biases);
    forward_pass<L3_LEN, OP_LEN>(floats_1,           floats_0, l3_op_weights, op_biases);

    return floats_0[0];
}
*/

Eval evaluate(Accumulator& accumulator, Pos& pos) {
    accumulator.recursively_update(pos);
    Eval color_multiple = (pos.turn() == WHITE) * 2 - 1;
    return accumulator.slice->white_ps * color_multiple;
}

}

inline Eval white_ps_value(const Color color, const Piece piece, const Square square) {
    Eval   color_multiple = (color == WHITE) * 2 - 1;
    Square square_flipped = square ^ (0b111000 * (color == BLACK));
    return nnue::ps_weights[piece][square_flipped] * color_multiple;
}


Accumulator::Accumulator() {
    slice = slice_stack;
}

void AccumulatorSlice::add_piece(const Color color, const Piece piece, const Square square) {
    white_ps += white_ps_value(color, piece, square);
}

void AccumulatorSlice::rem_piece(const Color color, const Piece piece, const Square square) {
    white_ps -= white_ps_value(color, piece, square);
}

void Accumulator::reset(const Pos& pos) {

    slice = slice_stack;
    slice->white_ps = 0;
    
    BB occupied = pos.pieces();

    while (occupied) {
        Square square = poplsb(occupied);
        Piece piece = pos.piece_on(square);
        Color color = pos.color_on(square);

        slice->add_piece(color, piece, square);
    }

    slice->accurate = true;

}

void Accumulator::recursively_update(const Pos& pos, size_t offset) {

    AccumulatorSlice* acc_slice =     slice - offset;
    const Slice*      pos_slice = pos.slice - offset;

    if (acc_slice->accurate)
        return;

    assert(acc_slice !=     slice_stack);
    assert(pos_slice != pos.slice_stack);
    
    if (!(acc_slice - 1)->accurate)
        recursively_update(pos, offset + 1);
    
	*acc_slice = *(acc_slice - 1);

    const Move   move        = pos_slice->move;
    const Piece  moving      = pos_slice->moving;
    const Piece  victim      = pos_slice->victim;
    
    const Square from_square = move::from_square(move);
	const Square to_square   = move::to_square(move);

    const Color turn = offset % 2 ? pos.turn() : pos.notturn();
    const Color notturn = !turn;
	
	acc_slice->rem_piece(turn, moving, from_square);

	if (move::is_capture(move)) {
		Square victim_square = move::capture_square(move);

		acc_slice->rem_piece(notturn, pos_slice->victim, victim_square);
	}

    Piece placed = move::is_promotion(move) ? move::promotion_piece(move) : moving;
	acc_slice->add_piece(turn, placed, to_square);

	if (move::is_king_castle(move)) {
		Square offset = (turn == BLACK) * (N_FILES * 7);
		Square rook_from = H1 + offset;
		Square rook_to   = F1 + offset;

		acc_slice->rem_piece(turn, ROOK, rook_from);
		acc_slice->add_piece(turn, ROOK, rook_to);
	}
	else if (move::is_queen_castle(move)) {
		Square offset = (turn == BLACK) * (N_FILES * 7);
		Square rook_from = A1 + offset;
		Square rook_to   = D1 + offset;

		acc_slice->rem_piece(turn, ROOK, rook_from);
		acc_slice->add_piece(turn, ROOK, rook_to);
	}
}

void Accumulator::push() {
    slice++;
    slice->accurate = false;
}

void Accumulator::pop() {
    slice--;
}
