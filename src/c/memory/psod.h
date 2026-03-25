/**
 *  memory/psod.h
 *
 *  Paged space object of size class d, 8192 words total, 8190 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_psod_h
#define __psse_memory_psod_h

#include <stdint.h>

#include "memory/header.h"

/**
 * @brief A paged space object of size class d, 8192 words total, 8190 words
 * payload.
 * 
 */
struct psod {
    struct pso_header header;
    union {
        char[65520] bytes;
         uint64_t[8190] words;
    } payload;
};

#endif
