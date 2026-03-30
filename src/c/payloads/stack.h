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
#include "memory/pso4.h"

#define STACKTAG "STK"
#define STACKTV  4936787

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
    uint32_t args;
    /** the depth of the stack below this frame */
    uint32_t depth;
};

struct pso_pointer fetch_arg( struct pso4 *frame, unsigned int index );

#endif
