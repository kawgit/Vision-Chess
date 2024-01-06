#include "bits.h"
#include "pos.h"
#include "hash.h"
#include "types.h"
#include <cstdlib>

using namespace std;

namespace zobrist {

    BB psqt[N_COLORS][N_PIECES][N_SQUARES];
    BB ep[N_SQUARES];
    BB cr[N_SQUARES];
    BB wtm;

    void init() {
        srand(38217392);

        for (Square square = A1; square < H8; square++) {
            
            for (Color color : {WHITE, BLACK})
                for (Piece piece = PAWN; piece <= KING; piece++)
                        psqt[color][piece][square] = rand_BB();

            ep[square] = rand_BB();

            cr[square] = rand_BB();
        }

        wtm = rand_BB();
    }
}