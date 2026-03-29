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
#include "payloads/free.h"

/**
 * @brief A paged space object of size class e, 16384 words total, 16382 words
 * payload.
 * 
 */
struct psoe {
    struct pso_header header;
    union {
        char bytes[131056];
        uint64_t words[16382];
        struct free_payload free;
    } payload;
};

#endif
