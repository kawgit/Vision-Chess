#include "types.h"
#include "pos.h"

extern Eval mat_points[6];

extern Eval piece_eval_maps[5][64];

extern Eval king_eval_map[64];

Eval evalPos(Pos &p, Eval LB, Eval UB);