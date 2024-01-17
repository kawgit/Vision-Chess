#pragma once

#include "types.h"
#include "bits.h"
#include "pos.h"



namespace zobrist {

    extern BB psqt[N_COLORS][N_PIECES][N_SQUARES];
    extern BB ep[N_SQUARES + 1];
    extern BB cr[N_SQUARES];
    extern BB wtm;

    void init();

}