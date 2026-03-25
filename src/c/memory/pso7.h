/**
 *  memory/pso7.h
 *
 *  Paged space object of size class 7, 128 words total, 126 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso7_h
#define __psse_memory_pso7_h

#include <stdint.h>

#include "memory/header.h"

/**
 * @brief A paged space object of size class 7, 128 words total, 126 words
 * payload.
 * 
 */
struct pso7 {
    struct pso_header header;
    union {
        char[1008] bytes;
         uint64_t[126] words;
    } payload;
};

#endif
