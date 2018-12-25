/*
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
 * Make an empty stack frame, and return it.
 * @param previous the current top-of-stack;
 * @param env the environment in which evaluation happens.
 * @return the new frame.
 */
struct stack_frame *make_empty_frame( struct stack_frame *previous,
                                      struct cons_pointer env ) {
    struct stack_frame *result = malloc( sizeof( struct stack_frame ) );
    /*
     * TODO: later, pop a frame off a free-list of stack frames
     */

    result->previous = previous;

    /*
     * clearing the frame with memset would probably be slightly quicker, but
     * this is clear.
     */
    result->more = NIL;
    result->function = NIL;

    for ( int i = 0; i < args_in_frame; i++ ) {
        set_reg( result, i, NIL );
    }

    return result;
}


/**
 * Allocate a new stack frame with its previous pointer set to this value,
 * its arguments set up from these args, evaluated in this env.
 * @param previous the current top-of-stack;
 * @args the arguments to load into this frame;
 * @param env the environment in which evaluation happens.
 * @return the new frame.
 */
struct stack_frame *make_stack_frame( struct stack_frame *previous,
                                      struct cons_pointer args,
                                      struct cons_pointer env,
                                      struct cons_pointer *exception ) {
    struct stack_frame *result = make_empty_frame( previous, env );

    for ( int i = 0; i < args_in_frame && consp( args ); i++ ) {
        /* iterate down the arg list filling in the arg slots in the
         * frame. When there are no more slots, if there are still args,
         * stash them on more */
        struct cons_space_object cell = pointer2cell( args );

        /*
         * TODO: if we were running on real massively parallel hardware,
         * each arg except the first should be handed off to another
         * processor to be evaled in parallel; but see notes here:
         * https://github.com/simon-brooke/post-scarcity/wiki/parallelism
         */
        struct stack_frame *arg_frame = make_empty_frame( result, env );
        set_reg( arg_frame, 0, cell.payload.cons.car );

        struct cons_pointer val = lisp_eval( arg_frame, env );
        if ( exceptionp( val ) ) {
            exception = &val;
            break;
        } else {
            set_reg( result, i, val );
        }

        free_stack_frame( arg_frame );

        args = cell.payload.cons.cdr;
    }
    if ( consp( args ) ) {
        /* if we still have args, eval them and stick the values on `more` */
        struct cons_pointer more = eval_forms( previous, args, env );
        result->more = more;
        inc_ref( more );
    }

    dump_frame( stderr, result );
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
    struct stack_frame *result = make_empty_frame( previous, env );

    for ( int i = 0; i < args_in_frame && !nilp( args ); i++ ) {
        /* iterate down the arg list filling in the arg slots in the
         * frame. When there are no more slots, if there are still args,
         * stash them on more */
        struct cons_space_object cell = pointer2cell( args );

        set_reg( result, i, cell.payload.cons.car );

        args = cell.payload.cons.cdr;
    }
    if ( consp( args ) ) {
        result->more = args;
        inc_ref( args );
    }

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
    if ( !nilp( frame->more ) ) {
        dec_ref( frame->more );
    }

    free( frame );
}


/**
 * Dump a stackframe to this stream for debugging
 * @param output the stream
 * @param frame the frame
 */
void dump_frame( FILE * output, struct stack_frame *frame ) {
    fputws( L"Dumping stack frame\n", output );
    for ( int arg = 0; arg < args_in_frame; arg++ ) {
        struct cons_space_object cell = pointer2cell( frame->arg[arg] );

        fwprintf( output, L"Arg %d:\t%c%c%c%c\tcount: %10u\tvalue: ", arg,
                  cell.tag.bytes[0],
                  cell.tag.bytes[1], cell.tag.bytes[2], cell.tag.bytes[3],
                  cell.count );

        print( output, frame->arg[arg] );
        fputws( L"\n", output );
    }
    fputws( L"More: \t", output );
    print( output, frame->more );
    fputws( L"\n", output );
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