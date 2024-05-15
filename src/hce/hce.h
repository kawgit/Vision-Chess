#pragma once

#include "../pos.h"

struct Evaluator {

    void reset(const Pos& pos) {};
    void push() {};
    void pop() {};
    Eval evaluate(const Pos& pos);

};