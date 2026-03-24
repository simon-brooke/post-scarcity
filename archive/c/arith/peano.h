/*
 * peano.h
 *
 * Basic peano arithmetic
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */


#ifndef PEANO_H
#define PEANO_H

#include "memory/consspaceobject.h"

/**
 * The maximum value we will allow in an integer cell: one less than 2^60:
 * (let ((s (make-string-output-stream)))
 *   (format s "0x0~XL" (- (expt 2 60) 1))
 *   (string-downcase (get-output-stream-string s)))
 *                              "0x0fffffffffffffffl"
 * 
 * So left shifting and right shifting by 60 bits is correct.
 */
#define MAX_INTEGER   ((__int128_t)0x0fffffffffffffffL)
#define INT_CELL_BASE ((__int128_t)MAX_INTEGER + 1) // ((__int128_t)0x1000000000000000L)

/**
 * @brief Number of value bits in an integer cell
 * 
 */
#define INTEGER_BIT_SHIFT (60)

/**
 * @brief return `true` if arg is `nil`,  else `false`. 
 *
 * Note that this doesn't really belong in `peano.h`, but after code cleanup it
 * was the last thing remaining in either `boolean.c` or `boolean.h`, and it 
 * wasn't worth keeping two files around for one one-line macro.
 * 
 * @param arg 
 * @return true if the sole argument is `nil`.
 * @return false otherwise.
 */
#define truthy(arg)(!nilp(arg))

bool zerop( struct cons_pointer arg );

struct cons_pointer negative( struct cons_pointer arg );

bool is_negative( struct cons_pointer arg );

struct cons_pointer absolute( struct cons_pointer arg );

long double to_long_double( struct cons_pointer arg );

int64_t to_long_int( struct cons_pointer arg );

struct cons_pointer lisp_absolute( struct stack_frame
                                   *frame, struct cons_pointer frame_pointer, struct
                                   cons_pointer env );

struct cons_pointer
lisp_add( struct stack_frame *frame, struct cons_pointer frame_pointer,
          struct cons_pointer env );

struct cons_pointer lisp_is_negative( struct stack_frame
                                      *frame,
                                      struct cons_pointer frame_pointer, struct
                                      cons_pointer env );

struct cons_pointer
lisp_multiply( struct stack_frame *frame,
               struct cons_pointer frame_pointer, struct cons_pointer env );

struct cons_pointer negative( struct cons_pointer arg );

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

struct cons_pointer lisp_ratio_to_real( struct stack_frame *frame,
                                        struct cons_pointer frame_pointer,
                                        struct cons_pointer env );

#endif /* PEANO_H */
