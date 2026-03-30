/**
 *  ops/stack_ops.h
 *
 *  Operations on a Lisp stack frame.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_ops_stack_ops_h
#define __psse_ops_stack_ops_h

#include "memory/pointer.h"
#include "memory/pso4.h"

/*
 * number of arguments stored in a stack frame
 */
#define args_in_frame 8

/** 
 * @brief The maximum depth of stack before we throw an exception.
 *
 * `0` is interpeted as `unlimited`.
 */
extern uint32_t stack_limit;

struct pso_pointer fetch_arg( struct pso4 *frame, unsigned int index );

#endif
