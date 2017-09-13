/**
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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add an indefinite number of numbers together
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer
lisp_add(struct stack_frame *frame, struct cons_pointer env);

/**
 * Multiply an indefinite number of numbers together
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer
lisp_multiply(struct stack_frame *frame, struct cons_pointer env);

/**
 * Subtract one number from another.
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer
lisp_subtract(struct stack_frame *frame, struct cons_pointer env);

#ifdef __cplusplus
}
#endif

#endif /* PEANO_H */

