/*
 * peano.h
 *
 * Basic peano arithmetic
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "consspaceobject.h"

#ifndef PEANO_H
#define PEANO_H

bool zerop( struct cons_pointer arg );

struct cons_pointer negative( struct cons_pointer frame,
                              struct cons_pointer arg );

bool is_negative( struct cons_pointer arg);

struct cons_pointer absolute( struct cons_pointer frame_pointer, struct cons_pointer arg);

long double to_long_double( struct cons_pointer arg );

struct cons_pointer lisp_absolute( struct stack_frame
                              *frame, struct cons_pointer frame_pointer, struct
                              cons_pointer env );

struct cons_pointer
lisp_add( struct stack_frame *frame, struct cons_pointer frame_pointer,
          struct cons_pointer env );

struct cons_pointer lisp_is_negative( struct stack_frame
                              *frame, struct cons_pointer frame_pointer, struct
                              cons_pointer env );

struct cons_pointer
lisp_multiply( struct stack_frame *frame,
               struct cons_pointer frame_pointer, struct cons_pointer env );

struct cons_pointer negative( struct cons_pointer frame,
                              struct cons_pointer arg );

struct cons_pointer subtract_2( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer arg1,
                                struct cons_pointer arg2 );

struct cons_pointer
lisp_subtract( struct stack_frame *frame,
               struct cons_pointer frame_pointer, struct cons_pointer env );

struct cons_pointer
lisp_divide( struct stack_frame *frame, struct cons_pointer frame_pointer,
             struct cons_pointer env );

#endif /* PEANO_H */
