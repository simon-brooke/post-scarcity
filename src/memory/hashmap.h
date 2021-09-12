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

#define DFLT_HASHMAP_BUCKETS 32

uint32_t get_hash( struct cons_pointer ptr );

void free_hashmap( struct cons_pointer ptr );

void dump_map( URL_FILE * output, struct cons_pointer pointer );

struct cons_pointer hashmap_get( struct cons_pointer mapp,
                                 struct cons_pointer key );

struct cons_pointer hashmap_put( struct cons_pointer mapp,
                                 struct cons_pointer key,
                                 struct cons_pointer val );

struct cons_pointer lisp_get_hash( struct stack_frame *frame,
                                   struct cons_pointer frame_pointer,
                                   struct cons_pointer env );

struct cons_pointer hashmap_keys( struct cons_pointer map );

struct cons_pointer lisp_hashmap_put( struct stack_frame *frame,
                                      struct cons_pointer frame_pointer,
                                      struct cons_pointer env );

struct cons_pointer lisp_hashmap_put_all( struct stack_frame *frame,
                                          struct cons_pointer frame_pointer,
                                          struct cons_pointer env );

struct cons_pointer lisp_make_hashmap( struct stack_frame *frame,
                                       struct cons_pointer frame_pointer,
                                       struct cons_pointer env );

struct cons_pointer make_hashmap( uint32_t n_buckets,
                                  struct cons_pointer hash_fn,
                                  struct cons_pointer write_acl );

#endif
