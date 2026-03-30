/**
 *  payloads/stack.h
 *
 *  a Lisp stack frame.
 *
 *  Sits in a pso4.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_stack_h
#define __psse_payloads_stack_h

#include "memory/pointer.h"

/*
 * number of arguments stored in a stack frame
 */
#define args_in_frame 8

/**
 * A stack frame. 
 */
struct stack_frame_payload {
    /** the previous frame. */
    struct pso_pointer previous;
    /** first 8 arument bindings. */
    struct pso_pointer arg[args_in_frame];
    /** list of any further argument bindings. */
    struct pso_pointer more;
    /** the function to be called. */
    struct pso_pointer function;
    /** the number of arguments provided. */
    uint32_t args;
    /** the depth of the stack below this frame */
    uint32_t depth;
};

#endif
