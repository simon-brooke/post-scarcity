/**
 *  memory/pso2.h
 *
 *  Paged space object of size class 2, four words total, two words payload.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso2_h
#define __psse_memory_pso2_h

#include <stdint.h>

#include "../payloads/psse-string.h"
#include "memory/header.h"
#include "payloads/cons.h"
#include "payloads/free.h"
#include "payloads/function.h"
#include "payloads/integer.h"
#include "payloads/keyword.h"
#include "payloads/lambda.h"
#include "payloads/nlambda.h"
#include "payloads/read_stream.h"
#include "payloads/special.h"
#include "payloads/symbol.h"
// #include "payloads/time.h"
#include "payloads/vector_pointer.h"
#include "payloads/write_stream.h"

/**
 * @brief A paged space object of size class 2, four words total, two words
 * payload.
 * 
 */
struct pso2 {
    struct pso_header header;
    union {
        char bytes[16];
        uint64_t words[2];
        struct cons_payload cons;
        struct free_payload free;
        struct function_payload function;
        struct integer_payload integer;
        struct lambda_payload lambda;
//        struct special_payload special;
        struct stream_payload stream;
//        struct time_payload time;
        struct vectorp_payload vectorp;
    } payload;
};

#endif
