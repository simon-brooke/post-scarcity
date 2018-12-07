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
    struct stack_frame *frame = make_empty_frame( NULL, oblist );

    frame->arg[0] = stream_pointer;
    struct cons_pointer result = lisp_read( frame, oblist );
    free_stack_frame( frame );

    return result;
}

/**
 * Dummy up a Lisp eval call with its own stack frame.
 */
struct cons_pointer repl_eval( struct cons_pointer input ) {
    struct stack_frame *frame = make_empty_frame( NULL, oblist );

    frame->arg[0] = NIL /* input */ ;
    struct cons_pointer result = lisp_eval( frame, oblist );
    free_stack_frame( frame );

    return result;
}

/**
 * Dummy up a Lisp print call with its own stack frame.
 */
struct cons_pointer repl_print( struct cons_pointer stream_pointer,
                                struct cons_pointer value ) {
    struct stack_frame *frame = make_empty_frame( NULL, oblist );

    frame->arg[0] = value;
    frame->arg[1] = NIL /* stream_pointer */ ;
    struct cons_pointer result = lisp_print( frame, oblist );
    free_stack_frame( frame );

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
    struct cons_pointer input_stream = make_read_stream( in_stream );
    struct cons_pointer output_stream = make_write_stream( out_stream );

    while ( !feof( pointer2cell( input_stream ).payload.stream.stream ) ) {
        if ( show_prompt ) {
            fwprintf( out_stream, L"\n:: " );
        }

        struct cons_pointer val = repl_eval( repl_read( input_stream ) );

        /* suppress the 'end of stream' exception */
        if ( exceptionp( val ) &&
             !feof( pointer2cell( input_stream ).payload.stream.stream ) ) {
            repl_print( output_stream, val );
        }
    }
}
