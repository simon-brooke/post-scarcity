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
#include <signal.h>

#include "memory/consspaceobject.h"
#include "debug.h"
#include "ops/intern.h"
#include "ops/lispops.h"
#include "memory/stack.h"

/**
 * @brief Handle an interrupt signal.
 * 
 * @param dummy 
 */
void int_handler(int dummy) {
    wprintf(L"TODO: handle ctrl-C in a more interesting way\n");
}

/**
 * The read/eval/print loop.
 */
void repl(  ) {
    signal(SIGINT, int_handler);
    debug_print( L"Entered repl\n", DEBUG_REPL );

    struct cons_pointer env =
        consp( oblist ) ? oblist : make_cons( oblist, NIL );

    /* bottom of stack */
    struct cons_pointer frame_pointer = make_stack_frame( NIL, NIL, env );

    if ( !nilp( frame_pointer ) ) {
        lisp_repl( get_stack_frame( frame_pointer ), frame_pointer, env );

        dec_ref( frame_pointer );
    }

    debug_print( L"Leaving repl\n", DEBUG_REPL );
}
