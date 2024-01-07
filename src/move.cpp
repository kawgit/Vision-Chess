#include "move.h"
#include "types.h"
#include "util.h"

namespace move {
    
    Square capture_square(Move move) {
        Square from_square_ = from_square(move);
        Square to_square_   = to_square(move);
        return is_ep(move) ? square_of(rank_of(from_square_), file_of(to_square_)) : to_square_;
    }

}