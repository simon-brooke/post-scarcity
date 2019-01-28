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
 * set a register in a stack frame. Alwaye use this to do so,
 * because that way we can be sure the inc_ref happens!
 */
void set_reg( struct stack_frame *frame, int reg, struct cons_pointer value ) {
    debug_printf( DEBUG_STACK, L"Setting register %d to ", reg );
    debug_print_object( value, DEBUG_STACK );
    debug_println( DEBUG_STACK );
    dec_ref( frame->arg[reg] ); /* if there was anything in that slot
                                 * previously other than NIL, we need to decrement it;
                                 * NIL won't be decremented as it is locked. */
    frame->arg[reg] = value;
    inc_ref( value );

    if ( reg == frame->args ) {
        frame->args++;
    }
}


/**
 * get the actual stackframe object from this `pointer`, or NULL if
 * `pointer` is not a stackframe pointer.
 */
struct stack_frame *get_stack_frame( struct cons_pointer pointer ) {
    struct stack_frame *result = NULL;
    struct vector_space_object *vso =
        pointer2cell( pointer ).payload.vectorp.address;

    if ( vectorpointp( pointer ) && stackframep( vso ) ) {
        result = ( struct stack_frame * ) &( vso->payload );
        debug_printf( DEBUG_STACK,
                      L"get_stack_frame: all good, returning %p\n", result );
    } else {
        debug_print( L"get_stack_frame: fail, returning NULL\n", DEBUG_STACK );
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

    if ( !nilp( result ) ) {
        struct stack_frame *frame = get_stack_frame( result );
        /*
         * \todo later, pop a frame off a free-list of stack frames
         */

        frame->previous = previous;

        /*
         * clearing the frame with memset would probably be slightly quicker, but
         * this is clear.
         */
        frame->more = NIL;
        frame->function = NIL;
        frame->args = 0;

        for ( int i = 0; i < args_in_frame; i++ ) {
            frame->arg[i] = NIL;
        }
    }
    debug_print( L"Leaving make_empty_frame\n", DEBUG_ALLOC );
    debug_dump_object( result, DEBUG_ALLOC );

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
    debug_print( L"Entering make_stack_frame\n", DEBUG_STACK );
    struct cons_pointer result = make_empty_frame( previous );

    if ( nilp( result ) ) {
        /* i.e. out of memory */
        result =
            make_exception( c_string_to_lisp_string( L"Memory exhausted." ),
                            previous );
    } else {
        struct stack_frame *frame = get_stack_frame( result );

        while ( frame->args < args_in_frame && consp( args ) ) {
            /* iterate down the arg list filling in the arg slots in the
             * frame. When there are no more slots, if there are still args,
             * stash them on more */
            struct cons_space_object cell = pointer2cell( args );

            /*
             * \todo if we were running on real massively parallel hardware,
             * each arg except the first should be handed off to another
             * processor to be evaled in parallel; but see notes here:
             * https://github.com/simon-brooke/post-scarcity/wiki/parallelism
             */
            struct cons_pointer val =
                eval_form( frame, result, cell.payload.cons.car, env );
            if ( exceptionp( val ) ) {
                result = val;
                break;
            } else {
                debug_printf( DEBUG_STACK, L"Setting argument %d to ",
                              frame->args );
                debug_print_object( cell.payload.cons.car, DEBUG_STACK );
                set_reg( frame, frame->args, val );
            }

            args = cell.payload.cons.cdr;
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

        }
    }
    debug_print( L"make_stack_frame: returning\n", DEBUG_STACK );
    debug_dump_object( result, DEBUG_STACK );

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
    debug_print( L"Entering make_special_frame\n", DEBUG_STACK );

    struct cons_pointer result = make_empty_frame( previous );

    if ( nilp( result ) ) {
        /* i.e. out of memory */
        result =
            make_exception( c_string_to_lisp_string( L"Memory exhausted." ),
                            previous );
    } else {
        struct stack_frame *frame = get_stack_frame( result );

        while ( frame->args < args_in_frame && !nilp( args ) ) {
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
        }
    }
    debug_print( L"make_special_frame: returning\n", DEBUG_STACK );
    debug_dump_object( result, DEBUG_STACK );

    return result;
}

/**
 * Free this stack frame.
 */
void free_stack_frame( struct stack_frame *frame ) {
    /*
     * \todo later, push it back on the stack-frame freelist
     */
    debug_print( L"Entering free_stack_frame\n", DEBUG_ALLOC );
    for ( int i = 0; i < args_in_frame; i++ ) {
        dec_ref( frame->arg[i] );
    }
    if ( !nilp( frame->more ) ) {
        dec_ref( frame->more );
    }
    debug_print( L"Leaving free_stack_frame\n", DEBUG_ALLOC );
}


/**
 * Dump a stackframe to this stream for debugging
 * @param output the stream
 * @param frame_pointer the pointer to the frame
 */
void dump_frame( URL_FILE * output, struct cons_pointer frame_pointer ) {
    struct stack_frame *frame = get_stack_frame( frame_pointer );

    if ( frame != NULL ) {
        url_fwprintf( output, L"Stack frame with %d arguments:\n",
                      frame->args );
        for ( int arg = 0; arg < frame->args; arg++ ) {
            struct cons_space_object cell = pointer2cell( frame->arg[arg] );

            url_fwprintf( output, L"Arg %d:\t%c%c%c%c\tcount: %10u\tvalue: ",
                          arg, cell.tag.bytes[0], cell.tag.bytes[1],
                          cell.tag.bytes[2], cell.tag.bytes[3], cell.count );

            print( output, frame->arg[arg] );
            url_fputws( L"\n", output );
        }
        if ( !nilp( frame->more ) ) {
            url_fputws( L"More: \t", output );
            print( output, frame->more );
            url_fputws( L"\n", output );
        }
    }
}

void dump_stack_trace( URL_FILE * output, struct cons_pointer pointer ) {
    if ( exceptionp( pointer ) ) {
        print( output, pointer2cell( pointer ).payload.exception.message );
        url_fputws( L"\n", output );
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
