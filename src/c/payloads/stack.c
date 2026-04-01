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

#include <stdarg.h>

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/pso4.h"
#include "memory/tags.h"

/**
 * @brief Construct a stack frame with this `previous` pointer, and arguments
 * taken from the remaining arguments to this function, which should all be
 * struct pso_pointer.
 *
 * @return a pso_pointer to the stack frame.
 */
struct pso_pointer make_frame( struct pso_pointer previous, ... ) {
    va_list args;
    va_start( args, previous );
    int count = va_arg( args, int );

    struct pso_pointer frame_pointer = allocate( STACKTAG, 4 );
    struct pso4 *frame = ( struct pso4 * ) pointer_to_object( frame_pointer );

    frame->payload.stack_frame.previous = previous;

    // I *think* the count starts with the number of args, so there are
    // one fewer actual args. Need to test to verify this!
    count--;
    int cursor = 0;
    frame->payload.stack_frame.args = count;

    for ( ; cursor < count && cursor < args_in_frame; cursor++ ) {
        struct pso_pointer argument = va_arg( args, struct pso_pointer );

        frame->payload.stack_frame.arg[cursor] = inc_ref( argument );
    }
    if ( cursor < count ) {
        struct pso_pointer more_args = nil;

        for ( ; cursor < count; cursor++ ) {
            more_args = cons( va_arg( args, struct pso_pointer ), more_args );
        }

        // should be frame->payload.stack_frame.more = reverse( more_args), but
        // we don't have reverse yet. TODO: fix.
        frame->payload.stack_frame.more = more_args;
    } else {
        for ( ; cursor < args_in_frame; cursor++ ) {
            frame->payload.stack_frame.arg[cursor] = nil;
        }
    }

    return frame_pointer;
}

/**
 * @brief When a stack frame is freed, all its pointers must be decremented.
 *
 * Lisp calling conventions; one expected arg, the pointer to the object to
 * be destroyed.
 */
struct pso_pointer destroy_stack_frame( struct pso_pointer fp,
                                        struct pso_pointer env ) {
    if ( stackp( fp ) ) {
        struct pso4 *frame = pointer_to_pso4( fp );
        struct pso4 *casualty =
            pointer_to_pso4( frame->payload.stack_frame.arg[0] );

        dec_ref( casualty->payload.stack_frame.previous );
        dec_ref( casualty->payload.stack_frame.function );
        dec_ref( casualty->payload.stack_frame.more );

        for ( int i = 0; i < args_in_frame; i++ ) {
            dec_ref( casualty->payload.stack_frame.arg[0] );
        }

        casualty->payload.stack_frame.args = 0;
        casualty->payload.stack_frame.depth = 0;
    }

    return nil;
}
