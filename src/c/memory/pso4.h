/**
 *  memory/pso4.h
 *
 *  Paged space object of size class 4, 16 words total, 14 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso4_h
#define __psse_memory_pso4_h

#include <stdint.h>

#include "memory/header.h"
#include "memory/stack.h"

/**
 * @brief A paged space object of size class 4, 16 words total, 14 words
 * payload.
 * 
 */
struct pso4 {
    struct pso_header header;
    union {
        char[112] bytes;
         uint64_t[14] words;
        struct stack_frame_payload stack_frame;
    } payload;
};

#endif
