/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
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
    while ( !feof( in_stream ) ) {
        if ( show_prompt ) {
            fwprintf( out_stream, L"\n:: " );
        }
        struct cons_pointer input = read( in_stream );
        fwprintf( error_stream, L"\nread {%d,%d}=> ", input.page,
                  input.offset );
        print( error_stream, input );

        struct stack_frame *frame = make_empty_frame( NULL, oblist );
        frame->arg[0] = input;
        struct cons_pointer value = lisp_eval( frame, oblist );
        free_stack_frame( frame );
        // print( out_stream, input );
        fwprintf( out_stream, L"\n" );
        fwprintf( error_stream, L"\neval {%d,%d}=> ", input.page,
                  input.offset );
        print( out_stream, value );
    }
}
