/*
 * hashmap.h
 *
 * Basic implementation of a hashmap.
 *
 * (c) 2021 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_hashmap_h
#define __psse_hashmap_h

#include "arith/integer.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/vectorspace.h"

/**
 * The payload of a hashmap. The number of buckets is assigned at run-time,
 * and is stored in n_buckets. Each bucket is something ASSOC can consume: 
 * i.e. either an assoc list or a further hashmap.
 */
struct hashmap_payload {
    struct cons_pointer hash_fn;
    uint32_t n_buckets;
    uint32_t unused; /* for word alignment and possible later expansion */
    struct cons_pointer buckets[];
};

uint32_t get_hash(struct cons_pointer ptr);

struct cons_pointer lisp_get_hash(struct stack_frame *frame,
                                  struct cons_pointer frame_pointer,
                                  struct cons_pointer env);

struct cons_pointer make_hashmap( uint32_t n_buckets, struct cons_pointer hash_fn);

#endif