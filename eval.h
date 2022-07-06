#include "types.h"
#include "pos.h"

extern Eval mat_points[6];

extern Eval piece_eval_maps[5][64];

extern Eval king_eval_map[2][64];

inline Eval get_piece_eval(Piece p) {
    return mat_points[p - PAWN];
}

Eval evalPos(Pos& p, Eval LB, Eval UB);

Eval evalMat(Pos& p, Color c);