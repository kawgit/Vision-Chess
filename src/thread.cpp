#include <cassert>
#include <thread>
#include <iostream>
#include <algorithm>

#include "uci.h"
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
        assert(thread->current_state == IDLE);

    while (threads.size() < num_threads)
        threads.push_back(new Thread(this));

    while (threads.size() > num_threads) {
        delete threads.back();
        threads.pop_back();
    }

}

void Pool::go() {

    active = true;
    root_depth = 0;

    for (Thread* thread : threads)
        thread->requested_state = ACTIVE;
    
    std::thread manager (&Pool::manage, this);
    manager.detach();

}

void Pool::stop() {
    active = false;
}

void Pool::manage() {
    
    reset_nodes();
    Timestamp start_time = get_current_ms();
    Pos pos_copy = root_pos;

    std::vector<Move> pv;

    Timestamp         last_time  = 0;
    Depth             last_depth = -1;
    Eval              last_eval  = EVAL_MIN;
    std::vector<Move> last_pv;

    while (active && get_current_ms() - start_time < max_time) {
        sleep_ms(10);

        bool found;
        TTEntry* entry = tt->probe(pos_copy.hashkey(), found);

        if (!found)
            continue;

        Timestamp time  = get_current_ms() - start_time;
        Depth     depth = entry->depth;
        Eval      eval  = entry->eval;
                  pv    = tt->probe_pv(pos_copy);

        if (time > last_time + 3000
            || depth != last_depth
            || eval  != last_eval
            || pv    != last_pv) {

            uci::print("info depth " + std::to_string(depth)
                     + " score cp "  + std::to_string(eval)
                     + " time "      + std::to_string(time)
                     + " nodes "     + std::to_string(nodes())
                     + " nps "       + std::to_string(nodes() * 1000 / time)
                     + " hashfull "  + std::to_string(tt->hashfull())
                     + " pv "        + movelist_to_string(pv));

            last_time  = time;
            last_depth = depth;
            last_eval  = eval;
            if (pv.size()) last_pv = pv;
        
        }

        handled_assert_out:

        continue;

    }

    for (Thread* thread : threads)
        thread->requested_state = IDLE;

    active = false;

    std::cout << "bestmove " << (pv.size() ? move_to_string(pv[0]) : "(none)") << (pv.size() >= 2 ? " ponder " + move_to_string(pv[1]) : "") << std::endl;

}

void Pool::clear() {
    tt->clear();
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

    if (root_depth == DEPTH_MAX)
        return root_depth;

    depth_mutex.lock();
    Depth result = ++root_depth;
    depth_mutex.unlock();
    return result;
}

size_t Pool::nodes() {
    size_t nodes = 0;
    for (const Thread* thread : threads)
        nodes += thread->nodes;
    return nodes;
}

void Pool::reset_nodes() {
    for (Thread* thread : threads)
        thread->nodes = 0;
}


Thread::Thread(Pool* pool_) {

    current_state = IDLE;
    pool = pool_;
    tt = pool->tt;
    pthread = std::thread(&Thread::idle, this);

}

Thread::~Thread() {

    current_state = KILLED;
    pthread.join();

    std::cout << "thread joined" << std::endl;

}

void Thread::work() {

    Eval alpha = EVAL_MIN;
    Eval beta  = EVAL_MAX;
    Eval delta = 100;

    Timestamp start_time = get_current_ms();

    while (requested_state == ACTIVE) {
        Depth depth = pool->pop_depth();

        Eval eval = search<ROOT>(depth, alpha, beta);

        if (eval <= alpha || eval >= beta)
            delta += 50;
        else if (delta >= 50)
            delta -= 10;

        alpha = Eval(std::max(int(eval) - int(delta), int(EVAL_MIN)));
        beta  = Eval(std::min(int(eval) + int(delta), int(EVAL_MAX)));

        while (depth == DEPTH_MAX && requested_state == ACTIVE)
            sleep_ms(10);

        bool found;
        TTEntry* entry = tt->probe(pos.hashkey(), found);

        if (requested_state == ACTIVE && found) {

            uci::print("info depth " + std::to_string(entry->depth)
                     + " score cp "  + std::to_string(entry->eval)
                     + " time "      + std::to_string(get_current_ms() - start_time)
                     + " hashfull "  + std::to_string(tt->hashfull())
                     + " pv "        + movelist_to_string(tt->probe_pv(pos)));
        }

    }


}

void Thread::kill() {
    current_state = KILLED;
    return;
}

void Thread::idle() {

    while (current_state == IDLE) {
        sleep_ms(10);

        if (requested_state == ACTIVE)
            work();

        if (requested_state == KILLED)
            kill();
    }

}

void Thread::reset(Pos& new_pos) {
    pos = new_pos;
    evaluator.reset(new_pos);
}

void Thread::do_move(Move move) {
    pos.do_move(move);
    evaluator.push();
    history.push();
}

void Thread::undo_move() {
    pos.undo_move();
    evaluator.pop();
    history.pop();
}