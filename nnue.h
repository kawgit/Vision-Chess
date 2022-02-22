#pragma once

#include <string>
#include "types.h"

using namespace std;

typedef int8_t fp8_t;

#define FRACT_BITS 4
#define FIXED_POINT_ONE (1 << FRACT_BITS)
#define INT_TO_FIXED(x) ((x) << FRACT_BITS)
#define FLOAT_TO_FIXED(x) ((fp8_t)((x) * FIXED_POINT_ONE))
#define FIXED_TO_INT(x) ((x) >> FRACT_BITS)
#define FIXED_TO_FLOAT(x) (((float)(x)) / FIXED_POINT_ONE)

#define FIXED_ADD(x, y) ((x) + (y))
#define FIXED_SUB(x, y) ((x) - (y))

#define FIXED_MUL(x, y) ((x)*(y) >> FRACT_BITS)
#define FIXED_DIV(x, y) (((x)<<FRACT_BITS) / (y))


/*
	STRUCTURE:

	INPUT LAYER (IMAGINARY): ksq, square, piece
	HL1, 256, int8, sigmoid
	HL2, 64, int16, sigmoid
	HL3, 16, int16, relu
	HL4, 1, int16, relu

	OUTPUT = net(turn) - net(notturn)
*/
class NNUE {
	public:
	void load(string path);
	void save(string path);
	void mutate(float radius);
	
	void add_node_weights(Color c, int index);
	void subtract_node_weights(Color c, int index);
	void set_piece(Square turn_ksq, Square notturn_ksq, Color c, Square sq, Piece pc);
	void rem_piece(Square turn_ksq, Square notturn_ksq, Color c, Square sq, Piece pc);
	void load_pos(char* pos_ptr);

	float feedforward(fp8_t* HL1);
	Eval evaluate(Color turn);

	private:

	const static int INPUT_LEN = 64*64*12;
	//turn, ksq, square, piece
	const static int HL1_LEN = 256;
	const static int HL2_LEN = 64;
	const static int HL3_LEN = 16;
	const static int HL4_LEN = 1;

	const static int HL1_W_LEN = INPUT_LEN*HL1_LEN;
	const static int HL2_W_LEN = HL1_LEN*HL2_LEN;
	const static int HL3_W_LEN = HL2_LEN*HL3_LEN;
	const static int HL4_W_LEN = HL3_LEN*HL4_LEN;

	fp8_t HL1_WHITE[HL1_LEN];
	fp8_t HL1_BLACK[HL1_LEN];

	fp8_t HL1_W[HL1_LEN][INPUT_LEN];
	float HL2_W[HL2_LEN][HL1_LEN];
	float HL3_W[HL3_LEN][HL2_LEN];
	float HL4_W[HL4_LEN][HL3_LEN];
};