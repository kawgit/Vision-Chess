#include <vector>
#include <mutex>

#include "pos.h"
#include "tt.h"

struct Pool {
    
    TT* tt;

    std::vector<Thread> threads;

    void add_thread();

    void rem_thread();

    void set_num_threads(size_t num_threads);

    Pool(size_t num_threads, size_t tt_size);

    ~Pool();
};

struct Thread {
    
    bool active = false;
    BB nodes = 0;

    Accumulator accumulator;
    TT* tt;
    
    std::mutex idle_mutex;
    std::thread pthread;

    Thread(TT* tt_);

    void idle();

    void wake();

    void kill();

    void reset(Pos new_pos);

    Eval search(Depth depth);
};