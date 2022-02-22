#include "nnue.h"
#include "pos.h"
#include "types.h"
#include "bits.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

void NNUE::load(string path) {
	ifstream file;
	file.open(path, ios::binary);

	if (file.is_open()) {
		file.read((char*)this, sizeof(NNUE));
	}
	else {
		cout << "File " << path << " could not be opened." << endl;
	}

	file.close();
}

void NNUE::save(string path) {
	ofstream file;
	file.open(path, ios::binary);

	if (file.is_open()) {
		file.write((char*)this, sizeof(NNUE));
	}
	else {
		cout << "File " << path << " could not be opened." << endl;
	}

	file.close();
}

void NNUE::mutate(float radius) {
	const static int total = HL1_W_LEN + HL2_W_LEN + HL3_W_LEN + HL4_W_LEN;
	int i = rand() % total;
	if (i < HL1_W_LEN) 		{
		HL1_W[i/INPUT_LEN][i%INPUT_LEN] += FLOAT_TO_FIXED(randf(-radius, radius));
	}
	else if (i < HL2_W_LEN) {
		i -= HL1_W_LEN;
		HL2_W[i/HL1_LEN][i%HL1_LEN] += randf(-radius, radius);
	}
	else if (i < HL3_W_LEN) {
		i -= HL2_W_LEN;
		HL3_W[i/HL2_LEN][i%HL2_LEN] += randf(-radius, radius);
	}
	else if (i < HL4_W_LEN) {
		i -= HL3_W_LEN;
		HL4_W[i/HL3_LEN][i%HL3_LEN] += randf(-radius, radius);
	}
}

void NNUE::add_node_weights(Color c, int index) {
	if (c == WHITE)
		for (int r = 0; r < HL1_LEN; r++) HL1_WHITE[r] += HL1_W[r][index];
	else
		for (int r = 0; r < HL1_LEN; r++) HL1_BLACK[r] += HL1_W[r][index];
}

void NNUE::subtract_node_weights(Color c, int index) {
	if (c == WHITE)
		for (int r = 0; r < HL1_LEN; r++) HL1_WHITE[r] -= HL1_W[r][index];
	else
		for (int r = 0; r < HL1_LEN; r++) HL1_BLACK[r] -= HL1_W[r][index];
}

void NNUE::set_piece(Square turn_ksq, Square notturn_ksq, Color c, Square sq, Piece pc) {
	add_node_weights(				   c, turn_ksq*(64*12) + sq*12 + pc);
	add_node_weights(getOppositeColor(c), notturn_ksq*(64*12) + sq*12 + pc + 6);
}


void NNUE::rem_piece(Square turn_ksq, Square notturn_ksq, Color c, Square sq, Piece pc) {
	subtract_node_weights(				    c, turn_ksq*(64*12) + sq*12 + pc);
	subtract_node_weights(getOppositeColor(c), notturn_ksq*(64*12) + sq*12 + pc + 6);
}

void NNUE::load_pos(char* pos_ptr) {
	Pos pos = *((Pos*)pos_ptr);

	memset(HL1_WHITE, 0, sizeof(HL1_WHITE[0]) * HL1_LEN);
	memset(HL1_BLACK, 0, sizeof(HL1_BLACK[0]) * HL1_LEN);

	int white_ksq = lsb(pos.getPieceMask(WHITE, KING));
	int black_ksq = lsb(pos.getPieceMask(BLACK, KING));

	for (int sq = 0; sq < 64; sq++) {
		if (pos.getPieceAt(WHITE, sq)) 
			set_piece(white_ksq, black_ksq, WHITE, sq, pos.getPieceAt(WHITE, sq));
		else if (pos.getPieceAt(BLACK, sq)) 
			set_piece(black_ksq, white_ksq, BLACK, sq, pos.getPieceAt(BLACK, sq));
	}
}


float NNUE::feedforward(fp8_t* input) {
	fp8_t HL1[HL1_LEN];
	for (int r = 0; r < HL1_LEN; r++) {
		HL1[r] = FIXED_DIV(input[r], input[r] + 1 );
	}

	float HL2[HL2_LEN]; 
	for (int r = 0; r < HL2_LEN; r++) {
		for (int s = 0; s < HL1_LEN; s++) 
			HL2[r] += HL1[s] * HL2_W[r][s];
		HL2[r] = HL2[r] / (HL2[r] + 1);
	}

	float HL3[HL3_LEN]; 
	for (int r = 0; r < HL3_LEN; r++) {
		for (int s = 0; s < HL2_LEN; s++) 
			HL3[r] += HL2[s] * HL3_W[r][s];
		HL3[r] = abs(HL3[r]);
	}

	float output; 
	for (int s = 0; s < HL3_LEN; s++) 
		output += HL3[s] * HL4_W[0][s];
	output = abs(output);

	return output;
}

Eval NNUE::evaluate(Color turn) {
	if (turn == WHITE)
		return (Eval)(feedforward(HL1_WHITE) - feedforward(HL1_BLACK));
}
