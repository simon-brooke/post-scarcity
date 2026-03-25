/**
 *  memory/pso5.h
 *
 *  Paged space object of size class 5, 32 words total, 30 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso5_h
#define __psse_memory_pso5_h

#include <stdint.h>

#include "memory/header.h"

/**
 * @brief A paged space object of size class 5, 32 words total, 30 words
 * payload.
 * 
 */
struct pso5 {
    struct pso_header header;
    union {
        char[240] bytes;
         uint64_t[30] words;
    } payload;
};

#endif
