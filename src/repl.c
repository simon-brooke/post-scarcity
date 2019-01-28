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

#include "consspaceobject.h"
#include "debug.h"
#include "intern.h"
#include "lispops.h"
#include "stack.h"


/**
 * The read/eval/print loop.
 */
void repl(  ) {
    debug_print( L"Entered repl\n", DEBUG_REPL );

    struct cons_pointer frame_pointer = make_stack_frame( NIL, NIL, oblist );

    if ( !nilp( frame_pointer ) ) {
        inc_ref( frame_pointer );

        lisp_repl( get_stack_frame( frame_pointer ), frame_pointer, oblist );

        dec_ref( frame_pointer );
    }

    debug_print( L"Leaving repl\n", DEBUG_REPL );
}
