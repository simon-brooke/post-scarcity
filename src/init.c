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

void bind_function( char *name, struct cons_pointer ( *executable )
                     ( struct stack_frame *,
                       struct cons_pointer, struct cons_pointer ) ) {
    deep_bind( c_string_to_lisp_symbol( name ),
               make_function( NIL, executable ) );
}

void bind_special( char *name, struct cons_pointer ( *executable )
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

    while ( ( option = getopt( argc, argv, "pdcv:" ) ) != -1 ) {
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
#ifdef DEBUG
    fputws( L"About to initialise cons pages\n", stderr );
#endif
    initialise_cons_pages(  );

#ifdef DEBUG
    fputws( L"Initialised cons pages, about to bind\n", stderr );
#endif

    /*
     * privileged variables (keywords)
     */
    deep_bind( c_string_to_lisp_symbol( "nil" ), NIL );
    deep_bind( c_string_to_lisp_symbol( "t" ), TRUE );

    /*
     * primitive function operations
     */
    bind_function( "add", &lisp_add );
    bind_function( "apply", &lisp_apply );
    bind_function( "assoc", &lisp_assoc );
    bind_function( "car", &lisp_car );
    bind_function( "cdr", &lisp_cdr );
    bind_function( "cons", &lisp_cons );
    bind_function( "divide", &lisp_divide );
    bind_function( "eq", &lisp_eq );
    bind_function( "equal", &lisp_equal );
    bind_function( "eval", &lisp_eval );
    bind_function( "exception", &lisp_exception );
    bind_function( "multiply", &lisp_multiply );
    bind_function( "read", &lisp_read );
    bind_function( "oblist", &lisp_oblist );
    bind_function( "print", &lisp_print );
    bind_function( "progn", &lisp_progn );
    bind_function( "reverse", &lisp_reverse );
    bind_function( "set", &lisp_set );
    bind_function( "subtract", &lisp_subtract );
    bind_function( "throw", &lisp_exception );
    bind_function( "type", &lisp_type );

    bind_function( "+", &lisp_add );
    bind_function( "*", &lisp_multiply );
    bind_function( "-", &lisp_subtract );
    bind_function( "/", &lisp_divide );
    bind_function( "=", &lisp_equal );

    /*
     * primitive special forms
     */
    bind_special( "cond", &lisp_cond );
    bind_special( "lambda", &lisp_lambda );
    /* bind_special( "λ", &lisp_lambda ); */
    bind_special( "nlambda", &lisp_nlambda );
    bind_special( "progn", &lisp_progn );
    bind_special( "quote", &lisp_quote );
    bind_special( "set!", &lisp_set_shriek );

    repl( stdin, stdout, stderr, show_prompt );

    if ( dump_at_end ) {
        dump_pages( stdout );
    }

    return ( 0 );
}
