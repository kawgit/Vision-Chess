#include "bits.h"
#include "hash.h"
#include "types.h"

namespace zobrist {

    BB psqt[N_COLORS][N_PIECES][N_SQUARES];
    BB ep[N_SQUARES + 1];
    BB cr[N_SQUARES];
    BB wtm;

    void init() {
        srand(38217392);

        for (Square square = A1; square <= H8; square++) {
            
            for (Color color : {WHITE, BLACK})
                for (Piece piece = PAWN; piece <= KING; piece++)
                        psqt[color][piece][square] = rand64();

            ep[square] = rand64();
            cr[square] = rand64();
        }

        wtm = rand64();

        ep[SQUARE_NONE] = 0;
    }
}