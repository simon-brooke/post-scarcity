/**
 *  memory/pso6.h
 *
 *  Paged space object of size class 6, 64 words total, 62 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso6_h
#define __psse_memory_pso6_h

#include <stdint.h>

#include "memory/header.h"

/**
 * @brief A paged space object of size class 6, 64 words total, 62 words
 * payload.
 * 
 */
struct pso6 {
    struct pso_header header;
    union {
        char[496] bytes;
         uint64_t[62] words;
    } payload;
};

#endif
