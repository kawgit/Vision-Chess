#include <cstring>
#include <fstream>
#include <vector>
#include <math.h>
#include <iomanip>

#include "nnue.h"

using namespace std;

void forward_pass(char* input_layer, char* output_layer, char* weights, size_t input_size, size_t output_size) {
    for (int o = 0; o < output_size; o++) {
        *(output_layer + o) = 0;
        for (int i = 0; i < input_size; i++) {
            *(output_layer + o) += *(input_layer + i) * *(weights + o * input_size + i);
        }
    }
}

NNUE::NNUE() {
    memset(l1, 0, 2 * L1_LEN);
    memset(l2, 0, 2 * L2_LEN);
    memset(l3, 0, 2 * L3_LEN);
    memset(l4, 0, 2 * L4_LEN);
    memset(l5, 0, 2 * L5_LEN);

    memset(in_l1_w, 0, IN_L1_W_LEN);
    memset(l1_l2_w, 0, L1_L2_W_LEN);
    memset(l2_l3_w, 0, L2_L3_W_LEN);
    memset(l3_l4_w, 0, L3_L4_W_LEN);
    memset(l4_l5_w, 0, L4_L5_W_LEN);

    for (Piece pt = PAWN; pt <= KING; pt++) {
        int pt_i = pt - PAWN;
        
    }

    l1_l2_w[0] = 1;
    l2_l3_w[0] = 1;
    l3_l4_w[0] = 1;
    l4_l5_w[0] = 1;
}

NNUE::NNUE(string path) {
    memset(l1, 0, 2 * L1_LEN);
    memset(l2, 0, 2 * L2_LEN);
    memset(l3, 0, 2 * L3_LEN);
    memset(l4, 0, 2 * L4_LEN);
    memset(l5, 0, 2 * L5_LEN);
    if (path.length() == 0) {
        memset(in_l1_w, 0, IN_L1_W_LEN);
        memset(l1_l2_w, 0, L1_L2_W_LEN);
        memset(l2_l3_w, 0, L2_L3_W_LEN);
        memset(l3_l4_w, 0, L3_L4_W_LEN);
        memset(l4_l5_w, 0, L4_L5_W_LEN);
    }
    else {
        ifstream file;
        file.open(path, ios::binary);
        if (file.is_open()) {
            file.read(&(in_l1_w[0]), IN_L1_W_LEN);
            file.read(&(l1_l2_w[0]), L1_L2_W_LEN);
            file.read(&(l2_l3_w[0]), L2_L3_W_LEN);
            file.read(&(l3_l4_w[0]), L3_L4_W_LEN);
            file.read(&(l4_l5_w[0]), L4_L5_W_LEN);
            file.close();
        }
        else { // todo fix this shitty code later
            memset(in_l1_w, 0, IN_L1_W_LEN);
            memset(l1_l2_w, 0, L1_L2_W_LEN);
            memset(l2_l3_w, 0, L2_L3_W_LEN);
            memset(l3_l4_w, 0, L3_L4_W_LEN);
            memset(l4_l5_w, 0, L4_L5_W_LEN);
            cout << "failed to open nnue file " << path << endl;
        }
    }
}

void NNUE::save(string path) {
    ofstream file;
    file.open(path, ios::binary);
    file.write(&(in_l1_w[0]), IN_L1_W_LEN);
    file.write(&(l1_l2_w[0]), L1_L2_W_LEN);
    file.write(&(l2_l3_w[0]), L2_L3_W_LEN);
    file.write(&(l3_l4_w[0]), L3_L4_W_LEN);
    file.write(&(l4_l5_w[0]), L4_L5_W_LEN);
    file.close();
}

int NNUE::evaluate(Color c) {
    c-=BLACK;
    forward_pass(&(l1[c][0]), &(l2[c][0]), &(l1_l2_w[0]), L1_LEN, L2_LEN);
    forward_pass(&(l2[c][0]), &(l3[c][0]), &(l2_l3_w[0]), L2_LEN, L3_LEN);
    forward_pass(&(l3[c][0]), &(l4[c][0]), &(l3_l4_w[0]), L3_LEN, L4_LEN);
    forward_pass(&(l4[c][0]), &(l5[c][0]), &(l4_l5_w[0]), L4_LEN, L5_LEN);
    c+=BLACK;

    c = opp(c);

    c-=BLACK;
    forward_pass(&(l1[c][0]), &(l2[c][0]), &(l1_l2_w[0]), L1_LEN, L2_LEN);
    forward_pass(&(l2[c][0]), &(l3[c][0]), &(l2_l3_w[0]), L2_LEN, L3_LEN);
    forward_pass(&(l3[c][0]), &(l4[c][0]), &(l3_l4_w[0]), L3_LEN, L4_LEN);
    forward_pass(&(l4[c][0]), &(l5[c][0]), &(l4_l5_w[0]), L4_LEN, L5_LEN);
    c+=BLACK;

    c = opp(c);
    return l5[c - BLACK][0] - l5[opp(c) - BLACK][0];
}

void NNUE::add_blurry_bonus(int sq, int piece, int bonus) {
    int t_r = sq / 8;
    int t_c = sq % 8;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int rel_row = t_r - r;
            int rel_col = t_c - c;
            double dist = sqrt(rel_row * rel_row + rel_col * rel_col);
            in_l1_w[calc_w_index(rc(r, c), piece)] += bonus * exp(-(dist / 2) * (dist / 2));
        }
    }
}

void NNUE::print_maps() {
    for (Piece pt = PAWN; pt <= PAWN; pt++) {
        cout << endl;
        for (int r = 7; r >= 0; r--) {
            for (int c = 0; c < 8; c++) {
                cout << setw(5) << (int)in_l1_w[calc_w_index(rc(r, c), pt)] << " ";
            }
            cout << endl << endl;
        }
    }
}
