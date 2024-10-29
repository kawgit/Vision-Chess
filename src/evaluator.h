#pragma once

#if defined(USE_HCE)
    #include "hce/hce.h"
#elif defined(USE_NNUE)
    #include "nnue/nnue.h"
#else
    static_assert(false)
#endif