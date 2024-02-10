#include <cassert>
#include <thread>

#include "types.h"
#include "thread.h"
#include "pos.h"
#include "tt.h"


Pool::Pool(size_t num_threads, size_t tt_size) {
    tt = new TT(tt_size);
    set_num_threads(num_threads);
}

Pool::~Pool() {
    set_num_threads(0);
    delete tt;
}


void Pool::set_num_threads(size_t num_threads) {

    while (threads.size() < num_threads) threads.push_back(Thread(tt));
    while (threads.size() > num_threads) {
        assert(!threads.back().active);
        threads.pop_back();
    }

}

Thread(TT* tt_) : tt{tt_} {

    

}

void Thread::idle() {

}

void Thread::wake() {

}

void Thread::kill() {

}


void Thread::reset(Pos new_pos) {

}

Eval Thread::search(Depth depth) {

}