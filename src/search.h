#pragma once

#include <iostream>

#include "attacks.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "pos.h"
#include "thread.h"
#include "util.h"

template<bool DIVIDE>
BB perft(Pos& pos, Depth depth);