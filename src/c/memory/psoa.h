/**
 *  memory/psoa.h
 *
 *  Paged space object of size class a, 1024 words total, 1022 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_psoa_h
#define __psse_memory_psoa_h

#include <stdint.h>

#include "memory/header.h"
#include "payloads/free.h"

/**
 * @brief A paged space object of size class a, 1024 words total, 1022 words
 * payload.
 * 
 */
struct psoa {
    struct pso_header header;
    union {
        char bytes[8176];
        uint64_t words[1022];
        struct free_payload free;
    } payload;
};

#endif
