/**
 *  ops/eq.c
 *
 *  Post Scarcity Software Environment: eq.
 *
 *  Test for pointer equality.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/memory.h"
#include "memory/node.h"
#include "memory/pointer.h"
#include "payloads/stack.h"
#include "ops/truth.h"

/**
 * @brief Function; do these two pointers point to the same object? 
 *
 * Shallow, cheap equality.
 *
 * TODO: if either of these pointers points to a cache cell, then what
 * we need to check is the cached value, which is not so cheap. Ouch!
 *
 * @param a a pointer;
 * @param b another pointer;
 * @return `true` if they are the same, else `false`
 */
bool eq( struct pso_pointer a, struct pso_pointer b ) {
    return ( a.node == b.node && a.page == b.page && a.offset == b.offset );
}

/**
 * Function; do all arguments to this finction point to the same object? 
 *
 * Shallow, cheap equality.
 *
 * * (eq? args...)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return `t` if all args are pointers to the same object, else `nil`;
 */
struct pso_pointer lisp_eq( struct pso4 *frame,
                            struct pso_pointer frame_pointer,
                            struct pso_pointer env ) {
    struct pso_pointer result = t;

    if ( frame->args > 1 ) {
        for ( int b = 1; ( truep( result ) ) && ( b < frame->args ); b++ ) {
            result = eq( fetch_arg(frame, 0), fetch_arg( frame, b ) ) ? t : nil;
        }
    }

    return result;
}
