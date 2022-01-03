#include "types.h"
#include "pos.h"
#include "search.h"

extern Eval mat_points[6];

extern Eval piece_eval_maps[5][64];

extern Eval king_eval_map[2][64];

inline Eval getPieceEval(Piece p) {
    return mat_points[p];
}

Eval evalPos(Pos& p, Eval LB, Eval UB);

Eval evalMat(Pos& p, Color c);

Eval evalPawns(Pos& p, Color c);

Eval evalKingSafety(Pos& p, Color c);

vector<Move> order(Search& search, Pos& pos, vector<Move> unsorted, Move entry_move);