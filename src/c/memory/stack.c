/**
 *  memory/stack.c
 *
 *  The execution stack.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/stack.h"

/** 
 * @brief The maximum depth of stack before we throw an exception.
 *
 * `0` is interpeted as `unlimited`.
 */
uint32_t stack_limit = 0;

/**
 * Fetch a pointer to the value of the local variable at this index.
 */
struct cons_pointer fetch_arg( struct stack_frame *frame, unsigned int index ) {
    struct cons_pointer result = NIL;

    if ( index < args_in_frame ) {
        result = frame->arg[index];
    } else {
        struct cons_pointer p = frame->more;

        for ( int i = args_in_frame; i < index; i++ ) {
            p = pointer2cell( p ).payload.cons.cdr;
        }

        result = pointer2cell( p ).payload.cons.car;
    }

    return result;
}
