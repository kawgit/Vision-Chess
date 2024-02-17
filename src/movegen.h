#pragma once

#include "pos.h"
#include "types.h"
#include "bits.h"
#include "move.h"
#include <vector>
#include <cassert>



namespace movegen {

    template<GenType GENTYPE>
    std::vector<Move> generate(Pos& pos);

}