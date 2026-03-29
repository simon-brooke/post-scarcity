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
#include "payloads/free.h"

/**
 * @brief A paged space object of size class c, 4096 words total, 4094 words
 * payload.
 * 
 */
struct psoc {
    struct pso_header header;
    union {
        char bytes[32752];
        uint64_t words[4094];
        struct free_payload free;
    } payload;
};

#endif
