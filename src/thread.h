#pragma once

#include <mutex>
#include <thread>
#include <vector>

#include "evaluator.h"
#include "history.h"
#include "pos.h"
#include "timer.h"
#include "tt.h"

class Thread;

class Pool {

    public:

    Pos   root_pos;
    Depth root_depth;
    bool  active;
    
    Timestamp max_time;
    std::mutex depth_mutex;
    TT* tt;
    std::vector<Thread*> threads;

    Pool(size_t num_threads, size_t tt_size);

    ~Pool();

    void set_num_threads(size_t num_threads);

    void go();

    void stop();

    void manage();

    void clear();

    void reset(Pos& new_pos);

    Depth get_depth();
    Depth pop_depth();

    size_t nodes();
    void reset_nodes();

};


class Thread {

    public:
    
    Pool* pool;
    TT* tt;

    ThreadState requested_state;
    ThreadState current_state;
    size_t nodes = 0;
    Depth root_depth;
    Depth root_pos;

    Pos pos;
    Evaluator evaluator;

    History history;

    std::thread pthread;

    Thread(Pool* pool_);

    ~Thread();

    void reset(Pos& new_pos);
    
    void idle();
    
    void work();

    void kill();

    template<NodeType NODETYPE>
    Eval search(Depth depth, Eval alpha, Eval beta);

    void do_move(Move move);
    
    void undo_move();
};