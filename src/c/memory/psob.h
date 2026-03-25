/**
 *  memory/psob.h
 *
 *  Paged space object of size class b, 2048 words total, 2046 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_psob_h
#define __psse_memory_psob_h

#include <stdint.h>

#include "memory/header.h"

/**
 * @brief A paged space object of size class b, 2048 words total, 2046 words
 * payload.
 * 
 */
struct psob {
    struct pso_header header;
    union {
        char[16368] bytes;
         uint64_t[2046] words;
    } payload;
};

#endif
