/*
 * init.c
 *
 * Start up and initialise the environement - just enough to get working
 * and (ultimately) hand off to the executive.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

#include "version.h"
#include "conspage.h"
#include "consspaceobject.h"
#include "debug.h"
#include "intern.h"
#include "lispops.h"
#include "peano.h"
#include "print.h"
#include "repl.h"

// extern char *optarg; /* defined in unistd.h */

void bind_function( wchar_t *name, struct cons_pointer ( *executable )
                     ( struct stack_frame *,
                       struct cons_pointer, struct cons_pointer ) ) {
    deep_bind( c_string_to_lisp_symbol( name ),
               make_function( NIL, executable ) );
}

void bind_special( wchar_t *name, struct cons_pointer ( *executable )
                    ( struct stack_frame *,
                      struct cons_pointer, struct cons_pointer ) ) {
    deep_bind( c_string_to_lisp_symbol( name ),
               make_special( NIL, executable ) );
}

int main( int argc, char *argv[] ) {
    /*
     * attempt to set wide character acceptance on all streams
     */
    fwide( stdin, 1 );
    fwide( stdout, 1 );
    fwide( stderr, 1 );
    int option;
    bool dump_at_end = false;
    bool show_prompt = false;

    while ( ( option = getopt( argc, argv, "cpdv:" ) ) != -1 ) {
        switch ( option ) {
            case 'c':
                print_use_colours = true;
                break;
            case 'd':
                dump_at_end = true;
                break;
            case 'p':
                show_prompt = true;
                break;
            case 'v':
                verbosity = atoi( optarg );
                break;
            default:
                fwprintf( stderr, L"Unexpected option %c\n", option );
                break;
        }
    }

    if ( show_prompt ) {
        fwprintf( stdout,
                  L"Post scarcity software environment version %s\n\n",
                  VERSION );
    }

    debug_print( L"About to initialise cons pages\n", DEBUG_BOOTSTRAP );

    initialise_cons_pages(  );

    debug_print( L"Initialised cons pages, about to bind\n", DEBUG_BOOTSTRAP );

    /*
     * privileged variables (keywords)
     */
    deep_bind( c_string_to_lisp_symbol( L"nil" ), NIL );
    deep_bind( c_string_to_lisp_symbol( L"t" ), TRUE );

    /*
     * primitive function operations
     */
    bind_function( L"add", &lisp_add );
    bind_function( L"apply", &lisp_apply );
    bind_function( L"assoc", &lisp_assoc );
    bind_function( L"car", &lisp_car );
    bind_function( L"cdr", &lisp_cdr );
    bind_function( L"cons", &lisp_cons );
    bind_function( L"divide", &lisp_divide );
    bind_function( L"eq", &lisp_eq );
    bind_function( L"equal", &lisp_equal );
    bind_function( L"eval", &lisp_eval );
    bind_function( L"exception", &lisp_exception );
    bind_function( L"multiply", &lisp_multiply );
    bind_function( L"read", &lisp_read );
    bind_function( L"oblist", &lisp_oblist );
    bind_function( L"print", &lisp_print );
    bind_function( L"progn", &lisp_progn );
    bind_function( L"reverse", &lisp_reverse );
    bind_function( L"set", &lisp_set );
    bind_function( L"subtract", &lisp_subtract );
    bind_function( L"throw", &lisp_exception );
    bind_function( L"type", &lisp_type );

    bind_function( L"+", &lisp_add );
    bind_function( L"*", &lisp_multiply );
    bind_function( L"-", &lisp_subtract );
    bind_function( L"/", &lisp_divide );
    bind_function( L"=", &lisp_equal );

    /*
     * primitive special forms
     */
    bind_special( L"cond", &lisp_cond );
    bind_special( L"lambda", &lisp_lambda );
    // bind_special( L"λ", &lisp_lambda );
    bind_special( L"nlambda", &lisp_nlambda );
    // bind_special( L"nλ", &lisp_nlambda );
    bind_special( L"progn", &lisp_progn );
    bind_special( L"quote", &lisp_quote );
    bind_special( L"set!", &lisp_set_shriek );

  debug_print(L"Initialised oblist\n", DEBUG_BOOTSTRAP);
  debug_dump_object(oblist, DEBUG_BOOTSTRAP);

    repl( stdin, stdout, stderr, show_prompt );

  debug_print(L"Freeing oblist\n", DEBUG_BOOTSTRAP);
  debug_printf(DEBUG_BOOTSTRAP, L"Oblist has %u references\n", pointer2cell(oblist).count);
  debug_dump_object(oblist, DEBUG_BOOTSTRAP);
    dec_ref(oblist);
  debug_dump_object(oblist, DEBUG_BOOTSTRAP);


    if ( dump_at_end ) {
        dump_pages( stdout );
    }

    return ( 0 );
}
