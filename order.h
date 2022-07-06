#include "types.h"
#include "pos.h"
#include "search.h"
#include "tuner.h"
#include <vector>

using namespace std;


vector<Move> order(vector<Move>& unsorted_moves, Pos& pos, ThreadInfo& ti, SearchInfo& si, int& interesting);

bool keeps_tempo(Move& move, Pos& pos, ThreadInfo& ti);