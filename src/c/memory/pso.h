/**
 *  memory/pso.h
 *
 *  Paged space objects.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_pso_h
#define __psse_memory_pso_h

#include <stdint.h>

#include "memory/header.h"
#include "payloads/cons.h"
#include "payloads/free.h"
#include "payloads/function.h"
#include "payloads/integer.h"
#include "payloads/ketwod.h"
#include "payloads/lambda.h"
#include "payloads/mutex.h"
#include "payloads/nlambda.h"
#include "payloads/read_stream.h"
#include "payloads/special.h"
#include "payloads/stack.h"
#include "payloads/string.h"
#include "payloads/symbol.h"
#include "payloads/time.h"
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
        char[16] bytes;
         uint64_t[2] words;
        struct cons_payload cons;
        struct free_payload free;
        struct function_payload function;
        struct integer_payload integer;
        struct lambda_payload lambda;
        struct special_payload special;
        struct stream_payload stream;
        struct time_payload time;
        struct vectorp_payload vectorp;
    } payload;
};

/**
 * @brief A paged space object of size class 3, 8 words total, 6 words
 * payload.
 * 
 */
struct pso3 {
    struct pso_header header;
    union {
        char[48] bytes;
         uint64_t[6] words;
        struct exception_payload exception;
        struct free_payload free;
        struct mutex_payload mutex;
    } payload;
};

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
        struct free_payload free;
        struct stack_frame_payload stack_frame;
    } payload;
};

/**
 * @brief A paged space object of size class 5, 32 words total, 30 words
 * payload.
 * 
 */
struct pso5 {
    struct pso_header header;
    union {
        char[240] bytes;
        uint64_t[30] words;
        struct free_payload free;
    } payload;
};

/**
 * @brief A paged space object of size class 6, 64 words total, 62 words
 * payload.
 * 
 */
struct pso6 {
    struct pso_header header;
    union {
        char[496] bytes;
        uint64_t[62] words;
        struct free_payload free;
        struct hashtable_payload hashtable;
        struct namespace_payload namespace;
    } payload;
};

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
        struct free_payload free;
    } payload;
};

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
        struct free_payload free;
    } payload;
};

/**
 * @brief A paged space object of size class 9, 512 words total, 510 words
 * payload.
 * 
 */
struct pso9 {
    struct pso_header header;
    union {
        char[4080] bytes;
        uint64_t[510] words;
        struct free_payload free;
    } payload;
};

/**
 * @brief A paged space object of size class a, 1024 words total, 1022 words
 * payload.
 * 
 */
struct psoa {
    struct pso_header header;
    union {
        char[8176] bytes;
        uint64_t[1022] words;
        struct free_payload free;
    } payload;
};

/**
 * @brief A paged space object of size class b, 2048 words total, 2046 words
 * payload.
 * 
 */
struct psob {
    struct pso_header header;
    union {
        char[16368] bytes;
        uint64_t[2046] words;
        struct free_payload free;
    } payload;
};

/**
 * @brief A paged space object of size class c, 4096 words total, 4094 words
 * payload.
 * 
 */
struct psoc {
    struct pso_header header;
    union {
        char[32752] bytes;
        uint64_t[4094] words;
        struct free_payload free;
    } payload;
};

/**
 * @brief A paged space object of size class d, 8192 words total, 8190 words
 * payload.
 * 
 */
struct psod {
    struct pso_header header;
    union {
        char[65520] bytes;
        uint64_t[8190] words;
        struct free_payload free;
    } payload;
};

/**
 * @brief A paged space object of size class e, 16384 words total, 16382 words
 * payload.
 * 
 */
struct psoe {
    struct pso_header header;
    union {
        char[131056] bytes;
        uint64_t[16382] words;
        struct free_payload free;
    } payload;
};

/**
 * @brief A paged space object of size class f, 32768 words total, 32766 words
 * payload.
 * 
 */
struct psof {
    struct pso_header header;
    union {
        char[262128] bytes;
        uint64_t[32766] words;
        struct free_payload free;
    } payload;
};
