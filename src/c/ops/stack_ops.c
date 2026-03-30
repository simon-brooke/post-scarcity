/**
 *  payloads/stack.c
 *
 *  The execution stack.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/node.h"
#include "memory/pso2.h"
#include "memory/pso4.h"
#include "payloads/stack.h"

/** 
 * @brief The maximum depth of stack before we throw an exception.
 *
 * `0` is interpeted as `unlimited`.
 */
uint32_t stack_limit = 0;

/**
 * Fetch a pointer to the value of the local variable at this index.
 */
struct pso_pointer fetch_arg( struct pso4 *frame, unsigned int index ) {
    struct pso_pointer result = nil;

    // TODO check that the frame is indeed a frame!
    if ( index < args_in_frame ) {
        result = frame->payload.stack_frame.arg[index];
    } else {
        struct pso_pointer p = frame->payload.stack_frame.more;

        for ( int i = args_in_frame; i < index; i++ ) {
            p = pointer_to_object( p )->payload.cons.cdr;
        }

        result = pointer_to_object( p )->payload.cons.car;
    }

    return result;
}
