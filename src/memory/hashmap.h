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

uint32_t get_hash( struct cons_pointer ptr );

void free_hashmap( struct cons_pointer ptr );

struct cons_pointer lisp_get_hash( struct stack_frame *frame,
                                   struct cons_pointer frame_pointer,
                                   struct cons_pointer env );

struct cons_pointer lisp_hashmap_put( struct stack_frame *frame,
                                      struct cons_pointer frame_pointer,
                                      struct cons_pointer env );

struct cons_pointer lisp_hashmap_put_all( struct stack_frame *frame,
                                          struct cons_pointer frame_pointer,
                                          struct cons_pointer env );

struct cons_pointer lisp_make_hashmap( struct stack_frame *frame,
                                       struct cons_pointer frame_pointer,
                                       struct cons_pointer env );

#endif