#include "types.h"
#include "pos.h"
#include "search.h"
// #include "tuner.h"
#include "move.h"
#include <vector>

using namespace std;

void insert_to_sorted(Move move, Score score, vector<Move>& moves, vector<Score>& scores, int lb);

vector<Move> order(vector<Move>& unsorted_moves, Pos& pos, ThreadInfo& ti, SearchInfo& si, int& num_good, int& num_boring, int& num_bad, bool for_qsearch = false);

//bool keeps_tempo(Move& move, Pos& pos, ThreadInfo& ti);