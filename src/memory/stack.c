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
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdlib.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "debug.h"
#include "dump.h"
#include "lispops.h"
#include "print.h"
#include "stack.h"
#include "vectorspace.h"

/**
 * get the actual stackframe object from this `pointer`, or NULL if
 * `pointer` is not a stackframe pointer.
 */
struct stack_frame *get_stack_frame( struct cons_pointer pointer ) {
    struct stack_frame *result = NULL;
    debug_print
        ( L"get_stack_frame: about to get a pointer to the vector space object\n",
          DEBUG_ALLOC );
    struct vector_space_object *vso =
        pointer2cell( pointer ).payload.vectorp.address;
    debug_print( L"get_stack_frame: got a pointer, about to test it\n",
                 DEBUG_ALLOC );

    if ( vectorpointp( pointer ) && stackframep( vso ) ) {
        debug_print
            ( L"get_stack_frame: pointer is good, about to set the result\n",
              DEBUG_ALLOC );

        result = ( struct stack_frame * ) &( vso->payload );
        fwprintf( stderr, L"get_stack_frame: all good, returning %p\n",
                  result );
    } else {
        debug_print( L"get_stack_frame: fail, returning NULL\n", DEBUG_ALLOC );
    }

    return result;
}

/**
 * Make an empty stack frame, and return it.
 * @param previous the current top-of-stack;
 * @param env the environment in which evaluation happens.
 * @return the new frame, or NULL if memory is exhausted.
 */
struct cons_pointer make_empty_frame( struct cons_pointer previous ) {
    debug_print( L"Entering make_empty_frame\n", DEBUG_ALLOC );
    struct cons_pointer result =
        make_vso( STACKFRAMETAG, sizeof( struct stack_frame ) );

    debug_dump_object( result, DEBUG_ALLOC );

    fwprintf( stderr,
              L"make_empty_frame: got vector_space_object with size %lu, tag %4.4s\n",
              pointer_to_vso( result )->header.size,
              &pointer_to_vso( result )->header.tag.bytes );

    if ( !nilp( result ) ) {
        debug_print( L"make_empty_frame: about to call get_stack_frame\n",
                     DEBUG_ALLOC );
        struct stack_frame *frame = get_stack_frame( result );
        /*
         * TODO: later, pop a frame off a free-list of stack frames
         */

        fwprintf( stderr,
                  L"make_empty_frame: about to set previous to %4.4s\n",
                  &pointer2cell( previous ).tag.bytes );
        frame->previous = previous;
        debug_print( L"make_empty_frame: about to call inc_ref\n",
                     DEBUG_ALLOC );
        inc_ref( previous );

        /*
         * clearing the frame with memset would probably be slightly quicker, but
         * this is clear.
         */
        frame->more = NIL;
        frame->function = NIL;
        frame->args = 0;

        debug_print( L"make_empty_frame: about to initialise arg registers\n",
                     DEBUG_ALLOC );
        for ( int i = 0; i < args_in_frame; i++ ) {
            set_reg( frame, i, NIL );
        }
    }
    debug_print( L"Leaving make_empty_frame\n", DEBUG_ALLOC );

    return result;
}

/**
 * Allocate a new stack frame with its previous pointer set to this value,
 * its arguments set up from these args, evaluated in this env.
 * @param previous the current top-of-stack;
 * @args the arguments to load into this frame;
 * @param env the environment in which evaluation happens.
 * @return the new frame, or an exception if one occurred while building it.
 */
struct cons_pointer make_stack_frame( struct cons_pointer previous,
                                      struct cons_pointer args,
                                      struct cons_pointer env ) {
    debug_print( L"Entering make_stack_frame\n", DEBUG_ALLOC );
    struct cons_pointer result = make_empty_frame( previous );

    if ( nilp( result ) ) {
        /* i.e. out of memory */
        result =
            make_exception( c_string_to_lisp_string( "Memory exhausted." ),
                            previous );
    } else {
        struct stack_frame *frame = get_stack_frame( result );

        for ( frame->args = 0; frame->args < args_in_frame && consp( args );
              frame->args++ ) {
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
            struct cons_pointer arg_frame_pointer = make_empty_frame( result );
            inc_ref( arg_frame_pointer );

            if ( nilp( arg_frame_pointer ) ) {
                result =
                    make_exception( c_string_to_lisp_string
                                    ( "Memory exhausted." ), previous );
                break;
            } else {
                struct stack_frame *arg_frame =
                    get_stack_frame( arg_frame_pointer );
                set_reg( arg_frame, 0, cell.payload.cons.car );

                struct cons_pointer val =
                    lisp_eval( arg_frame, arg_frame_pointer, env );
                if ( exceptionp( val ) ) {
                    result = val;
                    break;
                } else {
                    set_reg( frame, frame->args, val );
                }

                dec_ref( arg_frame_pointer );

                args = cell.payload.cons.cdr;
            }
        }
        if ( !exceptionp( result ) ) {
            if ( consp( args ) ) {
                /* if we still have args, eval them and stick the values on `more` */
                struct cons_pointer more =
                    eval_forms( get_stack_frame( previous ), previous, args,
                                env );
                frame->more = more;
                inc_ref( more );
            }

            debug_dump_object( result, DEBUG_ALLOC );
        }
    }
    debug_print( L"Leaving make_stack_frame\n", DEBUG_ALLOC );

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
struct cons_pointer make_special_frame( struct cons_pointer previous,
                                        struct cons_pointer args,
                                        struct cons_pointer env ) {
    debug_print( L"Entering make_special_frame\n", DEBUG_ALLOC );

    struct cons_pointer result = make_empty_frame( previous );

    if ( nilp( result ) ) {
        /* i.e. out of memory */
        result =
            make_exception( c_string_to_lisp_string( "Memory exhausted." ),
                            previous );
    } else {
        struct stack_frame *frame = get_stack_frame( result );

        for ( frame->args = 0; frame->args < args_in_frame && !nilp( args );
              frame->args++ ) {
            /* iterate down the arg list filling in the arg slots in the
             * frame. When there are no more slots, if there are still args,
             * stash them on more */
            struct cons_space_object cell = pointer2cell( args );

            set_reg( frame, frame->args, cell.payload.cons.car );

            args = cell.payload.cons.cdr;
        }
        if ( !exceptionp( result ) ) {
            if ( consp( args ) ) {
                frame->more = args;
                inc_ref( args );
            }

            debug_dump_object( result, DEBUG_ALLOC );
        }
    }
    debug_print( L"Leaving make_special_frame\n", DEBUG_ALLOC );

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
 * @param frame_pointer the pointer to the frame
 */
void dump_frame( FILE * output, struct cons_pointer frame_pointer ) {
    struct stack_frame *frame = get_stack_frame( frame_pointer );

    if ( frame != NULL ) {
        for ( int arg = 0; arg < frame->args; arg++ ) {
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
}

void dump_stack_trace( FILE * output, struct cons_pointer pointer ) {
    if ( exceptionp( pointer ) ) {
        print( output, pointer2cell( pointer ).payload.exception.message );
        fwprintf( output, L"\n" );
        dump_stack_trace( output,
                          pointer2cell( pointer ).payload.exception.frame );
    } else {
        while ( vectorpointp( pointer )
                && stackframep( pointer_to_vso( pointer ) ) ) {
            dump_frame( output, pointer );
            pointer = get_stack_frame( pointer )->previous;
        }
    }
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
