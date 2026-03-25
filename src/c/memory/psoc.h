/**
 *  memory/psoc.h
 *
 *  Paged space object of size class c, 4096 words total, 4094 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_psoc_h
#define __psse_memory_psoc_h

#include <stdint.h>

#include "memory/header.h"

/**
 * @brief A paged space object of size class c, 4096 words total, 4094 words
 * payload.
 * 
 */
struct psoc {
    struct pso_header header;
    union {
        char[32752] bytes;
         uint64_t[4094] words;
    } payload;
};

#endif
