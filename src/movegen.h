#pragma once

#include "pos.h"
#include "types.h"
#include "bits.h"
#include "move.h"
#include <vector>
#include <cassert>

using namespace std;

namespace movegen {

    enum GenType {

        FLAG_QUIETS    = 0b0001, // garuantees all quiet moves will be generated, abiding by legal flag
        FLAG_TACTICALS = 0b0010, // garuantees all captures and promotions will be generated, abiding by legal flag
        FLAG_CHECKS    = 0b0100, // garuantees all checks will be generated, regardless of quiet and capture flags, abiding by legal flag (TODO)
        FLAG_LEGAL     = 0b1000, // garuantees all moves will be legal, i.e. not placing our king in check

        PSEUDO          = FLAG_QUIETS | FLAG_TACTICALS | FLAG_CHECKS,
        LOUDS           = FLAG_LEGAL  | FLAG_TACTICALS | FLAG_CHECKS,
        ALL             = FLAG_QUIETS | FLAG_TACTICALS | FLAG_CHECKS | FLAG_LEGAL,
    };

    template<GenType GENTYPE>
    vector<Move> generate(Pos& pos);

}