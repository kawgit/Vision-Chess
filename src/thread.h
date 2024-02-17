#pragma once

#include <vector>
#include <mutex>
#include <thread>

#include "pos.h"
#include "tt.h"
#include "nnue.h"
#include "history.h"

// struct SliceData {

// }

class Thread;

class Pool {

    public:

    Pos   root_pos;
    Depth root_depth;
    
    std::mutex depth_mutex;
    TT* tt;
    std::vector<Thread*> threads;

    Pool(size_t num_threads, size_t tt_size);

    ~Pool();

    void set_num_threads(size_t num_threads);

    void wake();

    void manage();

    void reset(Pos& new_pos);

    Depth get_depth();
    Depth pop_depth();

};


class Thread {

    public:
    
    Pool* pool;
    TT* tt;

    ThreadState state;
    BB nodes = 0;
    Depth root_depth;
    Depth root_pos;

    Pos pos;
    Accumulator accumulator;
    History history;

    std::mutex state_mutex;
    std::thread pthread;


    Thread(Pool* pool_);

    ~Thread();

    ThreadState get_state();

    void set_state(ThreadState state);

    void reset(Pos& new_pos);
    
    void idle();
    
    void work();

    template<NodeType NODETYPE>
    Eval search(Depth depth, Eval alpha, Eval beta);

    template<NodeType NODETYPE>
    Eval qsearch(Eval alpha, Eval beta);

    void do_move(Move move);
    
    void undo_move();
};