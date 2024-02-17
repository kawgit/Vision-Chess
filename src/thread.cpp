#include <cassert>
#include <thread>
#include <iostream>

#include "types.h"
#include "thread.h"
#include "pos.h"
#include "tt.h"
#include "timer.h"


Pool::Pool(size_t num_threads, size_t tt_size) {
    tt = new TT(tt_size);
    set_num_threads(num_threads);
}

Pool::~Pool() {
    set_num_threads(0);
    delete tt;
}

void Pool::set_num_threads(size_t num_threads) {

    for (Thread* thread : threads)
        assert(thread->get_state() == IDLE);

    while (threads.size() < num_threads)
        threads.push_back(new Thread(this));

    while (threads.size() > num_threads) {
        delete threads.back();
        threads.pop_back();
    }

}

void Pool::wake() {

    root_depth = 0;

    for (Thread* thread : threads)
        thread->set_state(ACTIVE);
    
    std::thread manager (&Pool::manage, this);
    manager.detach();

}

void Pool::manage() {
    
    Timestamp start_time = get_current_ms();
    Pos pos_copy = root_pos;

    std::cout << size_t(pos_copy.hashkey()) << std::endl;
    std::cout << size_t(root_pos.hashkey()) << std::endl;
    
    while (get_current_ms() - start_time < 5000) {
        sleep_ms(100);
        bool found = false;
        const TTEntry* entry = tt->probe(pos_copy.hashkey(), found);

        if (found) {
            std::vector<Move> pv = tt->probe_pv(pos_copy);
            std::cout << "info" 
                    << " depth " << int(entry->depth)
                    << " eval "  << int(entry->eval)
                    << " pv "    << movelist_to_string(pv)
                    << " hashfull " << tt->hashfull() 
                    << std::endl;
        }
        else {
            std::cout << "info no entry found " << tt->hashfull() << std::endl;
        }
    }

    for (Thread* thread : threads)
        thread->state = IDLE;

}

void Pool::reset(Pos& new_pos) {
    root_pos = new_pos;
    for (Thread* thread : threads)
        thread->reset(new_pos);
}

Depth Pool::get_depth() {
    depth_mutex.lock();
    Depth result = root_depth;
    depth_mutex.unlock();
    return result;
}

Depth Pool::pop_depth() {
    depth_mutex.lock();
    Depth result = ++root_depth;
    depth_mutex.unlock();
    return result;
}

Thread::Thread(Pool* pool_) {

    set_state(IDLE);
    pool = pool_;
    tt = pool->tt;
    pthread = std::thread(&Thread::idle, this);

}

Thread::~Thread() {

    set_state(KILLED);
    pthread.join();

    std::cout << "thread joined" << std::endl;

}

ThreadState Thread::get_state() {
    state_mutex.lock();
    ThreadState result = state;
    state_mutex.unlock();
    return result;
}

void Thread::set_state(ThreadState new_state) {
    state_mutex.lock();
    state = new_state;
    state_mutex.unlock();
}

void Thread::work() {

    while (get_state() == ACTIVE) {
        Depth depth = pool->pop_depth();
        std::cout << "searching depth " << int(depth) << std::endl;
        Eval eval = search<ROOT>(depth, EVAL_MIN, EVAL_MAX);

        if (depth == DEPTH_MAX)
            set_state(IDLE);

    }

}

void Thread::idle() {

    while (true) {
        sleep_ms(10);

        if (get_state() == ACTIVE)
            work();

        if (get_state() == KILLED)
            return;
    }

}

void Thread::reset(Pos& new_pos) {
    pos = new_pos;
    accumulator.reset(new_pos);
}

void Thread::do_move(Move move) {
    pos.do_move(move);
    accumulator.push();
    history.push();
}

void Thread::undo_move() {
    pos.undo_move();
    accumulator.pop();
    history.pop();
}