#include "types.h"
#include "pos.h"
#include "timer.h"
#include "tt.h"
#include <vector>

using namespace std;

BB perft(Pos &p, Depth depth, bool divide);

struct Task {
    Pos p;
    Depth depth;
    Eval beta;
    Task(Pos &p_, Depth &max_depth_, Eval &beta_) {
        p = p_;
        depth = max_depth_;
        beta = beta_;
    }
};

class Search {
public:
    const static int num_threads = 1;

    const int RAZOR_DEPTH = 3;
    const Eval RAZOR_MARGIN = 100;

    const int FPRUNE_DEPTH = 3;
    const Eval FPRUNE_MARGIN = 950;

    const int MIN_LMR_DEPTH = 3;
    const int LMR_MARGIN = 3;

    Search(Pos &p);

    void go();
    void stop();
    Eval negaMax(Pos &p, Depth depth, Eval alpha, Eval beta);
    Eval quies(Pos &p, Eval alpha, Eval beta);

public:
    Timestamp begin_ms = 0;
    Timestamp min_ms = 0;
    Timestamp max_ms = ~0ULL;
    Depth max_depth = -1;

private:
    TT table;

    bool searching = false;
    int root_move_clock;
    Pos root_p;

    BB nodes = 0;
};