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

/**
 * \todo cannot throw an exception out of here, which is a problem.
 * if a ratio may legally have zero as a divisor, or something which is
 * not a number is passed in.
 */
long double to_long_double( struct cons_pointer arg );

/**
 * Add an indefinite number of numbers together
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer
lisp_add( struct stack_frame *frame, struct cons_pointer frame_pointer,
          struct cons_pointer env );

/**
 * Multiply an indefinite number of numbers together.
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer
lisp_multiply( struct stack_frame *frame,
               struct cons_pointer frame_pointer, struct cons_pointer env );

/**
 * return a cons_pointer indicating a number which is the
 * 0 - the number indicated by `arg`.
 */
struct cons_pointer negative( struct cons_pointer frame,
                              struct cons_pointer arg );

/**
 * return a cons_pointer indicating a number which is the result of
 * subtracting the numbers indicated by `arg2` from that indicated by `arg1`,
 * in the context of this `frame`.
 */
struct cons_pointer subtract_2( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer arg1,
                                struct cons_pointer arg2 );

/**
 * Subtract one number from another.
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer
lisp_subtract( struct stack_frame *frame,
               struct cons_pointer frame_pointer, struct cons_pointer env );

/**
 * Divide one number by another.
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer
lisp_divide( struct stack_frame *frame, struct cons_pointer frame_pointer,
             struct cons_pointer env );

#endif /* PEANO_H */
