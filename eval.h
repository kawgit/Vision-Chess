#include "types.h"
#include "pos.h"
#include <functional>

using namespace std;

extern Eval mat_points[6];

extern Eval piece_eval_maps[5][64];

extern Eval king_eval_map[2][64];

inline Eval get_piece_eval(Piece p) {
    return mat_points[p - PAWN];
}

inline int sqMapTrans(int sq) { return rc(7-(sq/8), sq%8); }

Eval eval_pos(Pos& p, Eval LB, Eval UB, bool debug = false);

Eval eval_mat(Pos& p, Color c);

struct Factor {
    string name;
    function<Eval(Pos&, Color)> func;

    Factor(string n, function<Eval(Pos&, Color)> f) {
        name = n;
        func = f;
    }
};