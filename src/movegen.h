#pragma once

#include <cassert>
#include <vector>

#include "bits.h"
#include "move.h"
#include "pos.h"
#include "types.h"



namespace movegen {

    template<GenType GENTYPE>
    std::vector<Move> generate(Pos& pos);

}