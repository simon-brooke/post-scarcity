/**
 *  memory/stack.h
 *
 *  The execution stack.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_stack_frame_h
#define __psse_payloads_stack_frame_h
#include <stdint.h>

#include "memory/pointer.h"
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
    int args;
    /** the depth of the stack below this frame */
    int depth;
};

#endif
