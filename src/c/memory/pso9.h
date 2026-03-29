/**
 *  memory/pso9.h
 *
 *  Paged space object of size class 9, 512 words total, 510 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso9_h
#define __psse_memory_pso9_h

#include <stdint.h>

#include "memory/header.h"
#include "payloads/free.h"

/**
 * @brief A paged space object of size class 9, 512 words total, 510 words
 * payload.
 * 
 */
struct pso9 {
    struct pso_header header;
    union {
        char bytes[4080];
        uint64_t words[510];
        struct free_payload free;
    } payload;
};

#endif
