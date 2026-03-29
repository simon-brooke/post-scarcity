/**
 *  memory/psof.h
 *
 *  Paged space object of size class f, 32768 words total, 32766 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_psof_h
#define __psse_memory_psof_h

#include <stdint.h>

#include "memory/header.h"
#include "payloads/free.h"

/**
 * @brief A paged space object of size class f, 32768 words total, 32766 words
 * payload.
 * 
 */
struct psof {
    struct pso_header header;
    union {
        char bytes[262128];
        uint64_t words[32766];
        struct free_payload free;
    } payload;
};

#endif
