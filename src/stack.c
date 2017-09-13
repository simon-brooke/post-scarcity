/**
 * stack.c
 *
 * The Lisp evaluation stack.
 *
 * Stack frames could be implemented in cons space; indeed, the stack
 * could simply be an assoc list consed onto the front of the environment.
 * But such a stack would be costly to search. The design sketched here,
 * with stack frames as special objects, SHOULD be substantially more
 * efficient, but does imply we need to generalise the idea of cons pages
 * with freelists to a more general 'equal sized object pages', so that
 * allocating/freeing stack frames can be more efficient.
 *
 * Stack frames are not yet a first class object; they have no VECP pointer
 * in cons space.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdlib.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "lispops.h"
#include "print.h"
#include "stack.h"

/**
 * Allocate a new stack frame with its previous pointer set to this value,
 * its arguments set up from these args, evaluated in this env.
 */
struct stack_frame *make_stack_frame( struct stack_frame *previous,
                                      struct cons_pointer args,
                                      struct cons_pointer env ) {
    /*
     * TODO: later, pop a frame off a free-list of stack frames
     */
    struct stack_frame *result = malloc( sizeof( struct stack_frame ) );

    result->previous = previous;

    /*
     * clearing the frame with memset would probably be slightly quicker, but
     * this is clear.
     */
    result->more = NIL;
    result->function = NIL;

    for ( int i = 0; i < args_in_frame; i++ ) {
        result->arg[i] = NIL;
    }

    for (int i = 0; i < args_in_frame && !nilp( args ); i++ ) {
      /* iterate down the arg list filling in the arg slots in the
       * frame. When there are no more slots, if there are still args,
       * stash them on more */
        struct cons_space_object cell = pointer2cell( args );

            /*
             * TODO: if we were running on real massively parallel hardware,
             * each arg except the first should be handed off to another
             * processor to be evaled in parallel
             */
            result->arg[i] = lisp_eval( cell.payload.cons.car, env, result );
            inc_ref( result->arg[i] );

            args = cell.payload.cons.cdr;
        }
            /*
             * TODO: this isn't right. These args should also each be evaled.
             */
            result->more = args;
            inc_ref( result->more );

    return result;
}

/**
 * A 'special' frame is exactly like a normal stack frame except that the
 * arguments are unevaluated.
 * @param previous the previous stack frame;
 * @param args a list of the arguments to be stored in this stack frame;
 * @param env the execution environment;
 * @return a new special frame.
 */
struct stack_frame *make_special_frame( struct stack_frame *previous,
                                      struct cons_pointer args,
                                      struct cons_pointer env ) {
    /*
     * TODO: later, pop a frame off a free-list of stack frames
     */
    struct stack_frame *result = malloc( sizeof( struct stack_frame ) );

    result->previous = previous;

    /*
     * clearing the frame with memset would probably be slightly quicker, but
     * this is clear.
     */
    result->more = NIL;
    result->function = NIL;

    for ( int i = 0; i < args_in_frame; i++ ) {
        result->arg[i] = NIL;
    }

    for (int i = 0; i < args_in_frame && !nilp( args ); i++ ) {
      /* iterate down the arg list filling in the arg slots in the
       * frame. When there are no more slots, if there are still args,
       * stash them on more */
        struct cons_space_object cell = pointer2cell( args );

        result->arg[i] = cell.payload.cons.car;
        inc_ref( result->arg[i] );

        args = cell.payload.cons.cdr;
    }
    result->more = args;
    inc_ref(args);

    return result;
}

/**
 * Free this stack frame.
 */
void free_stack_frame( struct stack_frame *frame ) {
    /*
     * TODO: later, push it back on the stack-frame freelist
     */
    for ( int i = 0; i < args_in_frame; i++ ) {
        dec_ref( frame->arg[i] );
    }
    dec_ref( frame->more );

    free( frame );
}

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
