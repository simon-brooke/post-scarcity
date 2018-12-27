/*
 * repl.c
 *
 * the read/eval/print loop
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>

#include "conspage.h"
#include "consspaceobject.h"
#include "dump.h"
#include "intern.h"
#include "lispops.h"
#include "read.h"
#include "print.h"
#include "stack.h"


/* TODO: this is subtly wrong. If we were evaluating
 *   (print (eval (read)))
 * then the stack frame for read would have the stack frame for
 * eval as parent, and it in turn would have the stack frame for
 * print as parent.
 */

/**
 * Dummy up a Lisp read call with its own stack frame.
 */
struct cons_pointer repl_read( struct cons_pointer stream_pointer ) {
    struct cons_pointer result = NIL;
    fputws( L"Entered repl_read\n", stderr );
    struct cons_pointer frame_pointer = make_empty_frame( NIL );
    fputws( L"repl_read: got stack_frame pointer\n", stderr );
    dump_object( stderr, frame_pointer );
    if ( !nilp( frame_pointer ) ) {
        inc_ref( frame_pointer );
        struct stack_frame *frame = get_stack_frame( frame_pointer );

        if ( frame != NULL ) {
            fputws( L"repl_read: about to set register\n", stderr );
            set_reg( frame, 0, stream_pointer );
            fputws( L"repl_read: about to read\n", stderr );
            struct cons_pointer result =
                lisp_read( frame, frame_pointer, oblist );
        }
        dec_ref( frame_pointer );
    }
    fputws( L"repl_read: returning\n", stderr );
    dump_object( stderr, result );

    return result;
}

/**
 * Dummy up a Lisp eval call with its own stack frame.
 */
struct cons_pointer repl_eval( struct cons_pointer input ) {
    fputws( L"Entered repl_eval\n", stderr );
    struct cons_pointer result = NIL;
    struct cons_pointer frame_pointer = make_empty_frame( NIL );
    if ( !nilp( frame_pointer ) ) {
        inc_ref( frame_pointer );
        struct stack_frame *frame = get_stack_frame( frame_pointer );

        if ( frame != NULL ) {
            set_reg( frame, 0, input );
            result = lisp_eval( frame, frame_pointer, oblist );
        }

        dec_ref( frame_pointer );
    }
    fputws( L"repl_eval: returning\n", stderr );
    dump_object( stderr, result );

    return result;
}

/**
 * Dummy up a Lisp print call with its own stack frame.
 */
struct cons_pointer repl_print( struct cons_pointer stream_pointer,
                                struct cons_pointer value ) {

    struct cons_pointer result = NIL;
    struct cons_pointer frame_pointer = make_empty_frame( NIL );
    if ( !nilp( frame_pointer ) ) {
        struct stack_frame *frame = get_stack_frame( frame_pointer );

        if ( frame != NULL ) {
            set_reg( frame, 0, value );
            set_reg( frame, 1, stream_pointer );
            result = lisp_print( frame, frame_pointer, oblist );
            free_stack_frame( frame );
        }
        dec_ref( frame_pointer );
    }

    return result;
}

/**
 * The read/eval/print loop
 * @param in_stream the stream to read from;
 * @param out_stream the stream to write to;
 * @param err_stream the stream to send errors to;
 * @param show_prompt true if prompts should be shown.
 */
void
repl( FILE * in_stream, FILE * out_stream, FILE * error_stream,
      bool show_prompt ) {
    fputws( L"Entered repl\n", stderr );
    struct cons_pointer input_stream = make_read_stream( in_stream );
    inc_ref( input_stream );

    struct cons_pointer output_stream = make_write_stream( out_stream );
    inc_ref( output_stream );
    while ( !feof( pointer2cell( input_stream ).payload.stream.stream ) ) {
        if ( show_prompt ) {
            fwprintf( out_stream, L"\n:: " );
        }

        struct cons_pointer input = repl_read( input_stream );
        inc_ref( input );

        if ( exceptionp( input ) ) {
            /* suppress the end-of-stream exception */
            if ( !feof( pointer2cell( input_stream ).payload.stream.stream ) ) {
                repl_print( output_stream, input );
            }
            break;
        } else {
            repl_print( output_stream, repl_eval( input ) );
        }
        dec_ref( input );
    }
}
