#pragma once

#include "types.h"

using namespace std;

inline Square switch_perspective(Square square) {
    return 8 * (7 - square / 8) + (square % 8);
}

inline int calc_w_index(Square s, Piece p) {
    return (p - PAWN) * 64 + s;
}

void forward_pass(char* input_layer, char* output_layer, char* weights, size_t input_size, size_t output_size);

const int IN_LEN = 64 * 13;
const int L1_LEN = 1;
const int L2_LEN = 1;
const int L3_LEN = 1;
const int L4_LEN = 1;
const int L5_LEN = 1;

const int IN_L1_W_LEN = IN_LEN * L1_LEN;
const int L1_L2_W_LEN = L1_LEN * L2_LEN;
const int L2_L3_W_LEN = L2_LEN * L3_LEN;
const int L3_L4_W_LEN = L3_LEN * L4_LEN;
const int L4_L5_W_LEN = L4_LEN * L5_LEN;

class NNUE {

    private:
    char in_l1_w[IN_L1_W_LEN];
    char l1_l2_w[L1_L2_W_LEN];
    char l2_l3_w[L2_L3_W_LEN];
    char l3_l4_w[L3_L4_W_LEN];
    char l4_l5_w[L4_L5_W_LEN];

    char in[2][L1_LEN];
    char l1[2][L1_LEN];
    char l2[2][L2_LEN];
    char l3[2][L3_LEN];
    char l4[2][L4_LEN];
    char l5[2][L5_LEN];

    public:

    NNUE();
    NNUE(string path);

    void save(string path);
    int evaluate(Color c);
    void add_blurry_bonus(int sq, int piece, int bonus);
    void print_maps();

    inline void add_piece(Color c, Square s, Piece p) {
        if (c == WHITE) {
            l1[WHITE - BLACK][0] += in_l1_w[calc_w_index(s, p)];
        }
        else {
            l1[BLACK - BLACK][0] += in_l1_w[calc_w_index(switch_perspective(s), p)];
        }
    }

	inline void rem_piece(Color c, Square s, Piece p) {
        if (c == WHITE) {
            l1[WHITE - BLACK][0] -= in_l1_w[calc_w_index(s, p)];
        }
        else {
            l1[BLACK - BLACK][0] -= in_l1_w[calc_w_index(switch_perspective(s), p)];
        }
    }
};

