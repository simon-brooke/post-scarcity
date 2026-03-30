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
#include "memory/pointer.h"
// #include "payloads/cons.h"
// #include "payloads/exception.h"
// #include "payloads/free.h"
// #include "payloads/function.h"
// #include "payloads/hashtable.h"
// #include "payloads/integer.h"
// #include "payloads/keyword.h"
// #include "payloads/lambda.h"
// #include "payloads/mutex.h"
// #include "payloads/namespace.h"
// #include "payloads/nlambda.h"
// #include "payloads/read_stream.h"
// #include "payloads/special.h"
// #include "payloads/stack.h"
// #include "payloads/string.h"
// #include "payloads/symbol.h"
// #include "payloads/time.h"
// #include "payloads/vector_pointer.h"
// #include "payloads/write_stream.h"

// /**
//  * @brief A paged space object of size class 2, four words total, two words
//  * payload.
//  * 
//  */
// struct pso2 {
//     struct pso_header header;
//     union {
//         char bytes[16];
//         uint64_t words[2];
//         struct cons_payload cons;
//         struct free_payload free;
//         struct function_payload function;
//         struct integer_payload integer;
//         struct lambda_payload lambda;
//         struct special_payload special;
//         struct stream_payload stream;
//         struct time_payload time;
//         struct vectorp_payload vectorp;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class 3, 8 words total, 6 words
//  * payload.
//  * 
//  */
// struct pso3 {
//     struct pso_header header;
//     union {
//         char bytes[48];
//         uint64_t words[6];
//         struct exception_payload exception;
//         struct free_payload free;
//         struct mutex_payload mutex;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class 4, 16 words total, 14 words
//  * payload.
//  * 
//  */
// struct pso4 {
//     struct pso_header header;
//     union {
//         char bytes[112];
//         uint64_t words[14];
//         struct free_payload free;
//         struct stack_frame_payload stack_frame;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class 5, 32 words total, 30 words
//  * payload.
//  * 
//  */
// struct pso5 {
//     struct pso_header header;
//     union {
//         char bytes[240];
//         uint64_t words[30];
//         struct free_payload free;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class 6, 64 words total, 62 words
//  * payload.
//  * 
//  */
// struct pso6 {
//     struct pso_header header;
//     union {
//         char bytes[496];
//         uint64_t words[62];
//         struct free_payload free;
//         struct hashtable_payload hashtable;
//         struct namespace_payload namespace;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class 7, 128 words total, 126 words
//  * payload.
//  * 
//  */
// struct pso7 {
//     struct pso_header header;
//     union {
//         char bytes[1008];
//         uint64_t words[126];
//         struct free_payload free;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class 8, 256 words total, 254 words
//  * payload.
//  * 
//  */
// struct pso8 {
//     struct pso_header header;
//     union {
//         char bytes[2032];
//         uint64_t words[254];
//         struct free_payload free;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class 9, 512 words total, 510 words
//  * payload.
//  * 
//  */
// struct pso9 {
//     struct pso_header header;
//     union {
//         char bytes[4080];
//         uint64_t words[510];
//         struct free_payload free;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class a, 1024 words total, 1022 words
//  * payload.
//  * 
//  */
// struct psoa {
//     struct pso_header header;
//     union {
//         char bytes[8176];
//         uint64_t words[1022];
//         struct free_payload free;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class b, 2048 words total, 2046 words
//  * payload.
//  * 
//  */
// struct psob {
//     struct pso_header header;
//     union {
//         char bytes[16368];
//         uint64_t words[2046];
//         struct free_payload free;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class c, 4096 words total, 4094 words
//  * payload.
//  * 
//  */
// struct psoc {
//     struct pso_header header;
//     union {
//         char bytes[32752];
//         uint64_t words[4094];
//         struct free_payload free;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class d, 8192 words total, 8190 words
//  * payload.
//  * 
//  */
// struct psod {
//     struct pso_header header;
//     union {
//         char bytes[65520];
//         uint64_t words[8190];
//         struct free_payload free;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class e, 16384 words total, 16382 words
//  * payload.
//  * 
//  */
// struct psoe {
//     struct pso_header header;
//     union {
//         char bytes[131056];
//         uint64_t words[16382];
//         struct free_payload free;
//     } payload;
// };

// /**
//  * @brief A paged space object of size class f, 32768 words total, 32766 words
//  * payload.
//  * 
//  */
// struct psof {
//     struct pso_header header;
//     union {
//         char bytes[262128];
//         uint64_t words[32766];
//         struct free_payload free;
//     } payload;
// };

struct pso_pointer allocate( char* tag, uint8_t size_class);

struct pso_pointer dec_ref( struct pso_pointer pointer );

struct pso_pointer inc_ref( struct pso_pointer pointer );

void lock_object( struct pso_pointer pointer);

uint32_t get_tag_value( struct pso_pointer pointer);

#endif
