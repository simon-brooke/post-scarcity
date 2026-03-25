/**
 *  payloads/function.h
 *
 *  an ordinary Lisp function - one whose arguments are pre-evaluated.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_function_h
#define __psse_payloads_function_h

#include "memory/pointer.h"

/**
 * @brief Tag for an ordinary Lisp function - one whose arguments are pre-evaluated.
 * \see LAMBDATAG for interpretable functions.
 * \see SPECIALTAG for functions whose arguments are not pre-evaluated.
 */
#define FUNCTIONTAG "FUN"

/**
 * @brief Payload of a function cell.
 * `source` points to the source from which the function was compiled, or NIL
 * if it is a primitive.
 * `executable` points to a function which takes a pointer to a stack frame
 * (representing its stack frame) and a cons pointer (representing its
 * environment) as arguments and returns a cons pointer (representing its
 * result).
 */
struct function_payload {
    /**
     * pointer to metadata (e.g. the source from which the function was compiled).
     */
    struct cons_pointer meta;
    /**  pointer to a function which takes a cons pointer (representing
     * its argument list) and a cons pointer (representing its environment) and a
     * stack frame (representing the previous stack frame) as arguments and returns
     * a cons pointer (representing its result).
     * \todo check this documentation is current!
     */
    struct cons_pointer ( *executable ) ( struct stack_frame *,
                                          struct cons_pointer,
                                          struct cons_pointer );
};

#endif
