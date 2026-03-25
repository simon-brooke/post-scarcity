/**
 *  memory/pso8.h
 *
 *  Paged space object of size class 8, 256 words total, 254 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso8_h
#define __psse_memory_pso8_h

#include <stdint.h>

#include "memory/header.h"

/**
 * @brief A paged space object of size class 8, 256 words total, 254 words
 * payload.
 * 
 */
struct pso8 {
    struct pso_header header;
    union {
        char[2032] bytes;
         uint64_t[254] words;
    } payload;
};

#endif
