/**
 *  memory/pso3.h
 *
 *  Paged space object of size class 3, 8 words total, 6 words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso3_h
#define __psse_memory_pso3_h

#include <stdint.h>

#include "memory/header.h"
#include "payloads/exception.h"
#include "payloads/free.h"
#include "payloads/mutex.h"


/**
 * @brief A paged space object of size class 3, 8 words total, 6 words
 * payload.
 * 
 */
struct pso3 {
    struct pso_header header;
    union {
        char bytes[48];
        uint64_t words[6];
        struct exception_payload exception;
        struct free_payload free;
        struct mutex_payload mutex;
    } payload;
};

#endif
