/**
 *  memory/psoe.h
 *
 *  Paged space object of size class e, 16384 words total, 16382 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_psoe_h
#define __psse_memory_psoe_h

#include <stdint.h>

#include "memory/header.h"

/**
 * @brief A paged space object of size class e, 16384 words total, 16382 words
 * payload.
 * 
 */
struct psoe {
    struct pso_header header;
    union {
        char[131056] bytes;
         uint64_t[16382] words;
    } payload;
};

#endif
