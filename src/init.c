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

#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

/* libcurl, used for io */
#include <curl/curl.h>

#include "version.h"
#include "conspage.h"
#include "consspaceobject.h"
#include "debug.h"
#include "hashmap.h"
#include "intern.h"
#include "io.h"
#include "lispops.h"
#include "meta.h"
#include "peano.h"
#include "print.h"
#include "repl.h"
#include "psse_time.h"

// extern char *optarg; /* defined in unistd.h */

/**
 * Bind this compiled `executable` function, as a Lisp function, to
 * this name in the `oblist`.
 * \todo where a function is not compiled from source, we could cache
 * the name on the source pointer. Would make stack frames potentially
 * more readable and aid debugging generally.
 */
void bind_function( wchar_t *name, struct cons_pointer ( *executable )
                     ( struct stack_frame *,
                       struct cons_pointer, struct cons_pointer ) ) {
    struct cons_pointer n = c_string_to_lisp_symbol( name );
    struct cons_pointer meta =
        make_cons( make_cons( c_string_to_lisp_keyword( L"primitive" ), TRUE ),
                   make_cons( make_cons( c_string_to_lisp_keyword( L"name" ),
                                         n ),
                              NIL ) );

    deep_bind( n, make_function( meta, executable ) );
}

/**
 * Bind this compiled `executable` function, as a Lisp special form, to
 * this `name` in the `oblist`.
 */
void bind_special( wchar_t *name, struct cons_pointer ( *executable )
                    ( struct stack_frame *,
                      struct cons_pointer, struct cons_pointer ) ) {
    struct cons_pointer n = c_string_to_lisp_symbol( name );
    struct cons_pointer meta =
        make_cons( make_cons( c_string_to_lisp_keyword( L"primitive" ), TRUE ),
                   make_cons( make_cons( c_string_to_lisp_keyword( L"name" ),
                                         n ),
                              NIL ) );

    deep_bind( n, make_special( meta, executable ) );
}

/**
 * Bind this `value` to this `name` in the `oblist`.
 */
void bind_value( wchar_t *name, struct cons_pointer value ) {
    struct cons_pointer n = c_string_to_lisp_symbol( name );
    inc_ref( n );

    deep_bind( n, value );

    dec_ref( n );
}

void print_banner(  ) {
    fwprintf( stdout, L"Post-Scarcity Software Environment version %s\n\n",
              VERSION );
}

/**
 * Print command line options to this `stream`.
 * 
 * @stream the stream to print to.
 */
void print_options( FILE * stream ) {
    fwprintf( stream, L"Expected options are:\n" );
    fwprintf( stream,
              L"\t-d\tDump memory to standard out at end of run (copious!);\n" );
    fwprintf( stream, L"\t-h\tPrint this message and exit;\n" );
    fwprintf( stream, L"\t-p\tShow a prompt (default is no prompt);\n" );
    fwprintf( stream,
              L"\t-v LEVEL\n\t\tSet verbosity to the specified level (0...512)\n" );
    fwprintf( stream, L"\t\tWhere bits are interpreted as follows:\n" );
    fwprintf( stream, L"\t\t1\tALLOC;\n" );
    fwprintf( stream, L"\t\t2\tARITH;\n" );
    fwprintf( stream, L"\t\t4\tBIND;\n" );
    fwprintf( stream, L"\t\t8\tBOOTSTRAP;\n" );
    fwprintf( stream, L"\t\t16\tEVAL;\n" );
    fwprintf( stream, L"\t\t32\tINPUT/OUTPUT;\n" );
    fwprintf( stream, L"\t\t64\tLAMBDA;\n" );
    fwprintf( stream, L"\t\t128\tREPL;\n" );
    fwprintf( stream, L"\t\t256\tSTACK.\n" );
}

/**
 * main entry point; parse command line arguments, initialise the environment,
 * and enter the read-eval-print loop.
 */
int main( int argc, char *argv[] ) {
    int option;
    bool dump_at_end = false;
    bool show_prompt = false;

    setlocale( LC_ALL, "" );
    if ( io_init(  ) != 0 ) {
        fputs( "Failed to initialise I/O subsystem\n", stderr );
        exit( 1 );
    }

    while ( ( option = getopt( argc, argv, "phdv:" ) ) != -1 ) {
        switch ( option ) {
            case 'd':
                dump_at_end = true;
                break;
            case 'h':
                print_banner(  );
                print_options( stdout );
                exit( 0 );
                break;
            case 'p':
                show_prompt = true;
                break;
            case 'v':
                verbosity = atoi( optarg );
                break;
            default:
                fwprintf( stderr, L"Unexpected option %c\n", option );
                print_options( stderr );
                exit( 1 );
                break;
        }
    }

    if ( show_prompt ) {
        print_banner(  );
    }

    debug_print( L"About to initialise cons pages\n", DEBUG_BOOTSTRAP );

    initialise_cons_pages(  );

    debug_print( L"Initialised cons pages, about to bind\n", DEBUG_BOOTSTRAP );

//    TODO: oblist-as-hashmap (which is what we ultimately need) is failing hooribly
//    oblist = inc_ref( make_hashmap( 32, NIL, TRUE ) );

    /*
     * privileged variables (keywords)
     */
    bind_value( L"nil", NIL );
    bind_value( L"t", TRUE );

    /*
     * standard input, output, error and sink streams
     * attempt to set wide character acceptance on all streams
     */
    URL_FILE *sink = url_fopen( "/dev/null", "w" );
    fwide( stdin, 1 );
    fwide( stdout, 1 );
    fwide( stderr, 1 );
    fwide( sink->handle.file, 1 );
    bind_value( L"*in*", make_read_stream( file_to_url_file( stdin ),
                                           make_cons( make_cons
                                                      ( c_string_to_lisp_keyword
                                                        ( L"url" ),
                                                        c_string_to_lisp_string
                                                        ( L"system:standard input" ) ),
                                                      NIL ) ) );
    bind_value( L"*out*",
                make_write_stream( file_to_url_file( stdout ),
                                   make_cons( make_cons
                                              ( c_string_to_lisp_keyword
                                                ( L"url" ),
                                                c_string_to_lisp_string
                                                ( L"system:standard output]" ) ),
                                              NIL ) ) );
    bind_value( L"*log*", make_write_stream( file_to_url_file( stderr ),
                                             make_cons( make_cons
                                                        ( c_string_to_lisp_keyword
                                                          ( L"url" ),
                                                          c_string_to_lisp_string
                                                          ( L"system:standard log" ) ),
                                                        NIL ) ) );
    bind_value( L"*sink*", make_write_stream( sink,
                                              make_cons( make_cons
                                                         ( c_string_to_lisp_keyword
                                                           ( L"url" ),
                                                           c_string_to_lisp_string
                                                           ( L"system:standard sink" ) ),
                                                         NIL ) ) );
    /*
     * the default prompt
     */
    bind_value( L"*prompt*",
                show_prompt ? c_string_to_lisp_symbol( L":: " ) : NIL );
    /*
     * primitive function operations
     */
    bind_function( L"absolute", &lisp_absolute );
    bind_function( L"add", &lisp_add );
    bind_function( L"append", &lisp_append );
    bind_function( L"apply", &lisp_apply );
    bind_function( L"assoc", &lisp_assoc );
    bind_function( L"car", &lisp_car );
    bind_function( L"cdr", &lisp_cdr );
    bind_function( L"close", &lisp_close );
    bind_function( L"cons", &lisp_cons );
    bind_function( L"divide", &lisp_divide );
    bind_function( L"eq", &lisp_eq );
    bind_function( L"equal", &lisp_equal );
    bind_function( L"eval", &lisp_eval );
    bind_function( L"exception", &lisp_exception );
    bind_function( L"get-hash", &lisp_get_hash );
    bind_function( L"hashmap", lisp_make_hashmap );
    bind_function( L"inspect", &lisp_inspect );
    bind_function( L"keys", &lisp_keys );
    bind_function( L"list", &lisp_list);
    bind_function( L"mapcar", &lisp_mapcar );
    bind_function( L"meta", &lisp_metadata );
    bind_function( L"metadata", &lisp_metadata );
    bind_function( L"multiply", &lisp_multiply );
    bind_function( L"negative?", &lisp_is_negative );
    bind_function( L"oblist", &lisp_oblist );
    bind_function( L"open", &lisp_open );
    bind_function( L"print", &lisp_print );
    bind_function( L"progn", &lisp_progn );
    bind_function( L"put", lisp_hashmap_put );
    bind_function( L"put-all", &lisp_hashmap_put_all );
    bind_function( L"read", &lisp_read );
    bind_function( L"read-char", &lisp_read_char );
    bind_function( L"repl", &lisp_repl );
    bind_function( L"reverse", &lisp_reverse );
    bind_function( L"set", &lisp_set );
    bind_function( L"slurp", &lisp_slurp );
    bind_function( L"source", &lisp_source );
    bind_function( L"subtract", &lisp_subtract );
    bind_function( L"throw", &lisp_exception );
    bind_function( L"time", &lisp_time );
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
    bind_special( L"\u03bb", &lisp_lambda );  // λ
    bind_special(L"let", &lisp_let);
    bind_special( L"nlambda", &lisp_nlambda );
    bind_special( L"n\u03bb", &lisp_nlambda );
    bind_special( L"progn", &lisp_progn );
    bind_special( L"quote", &lisp_quote );
    bind_special( L"set!", &lisp_set_shriek );
    debug_print( L"Initialised oblist\n", DEBUG_BOOTSTRAP );
    debug_dump_object( oblist, DEBUG_BOOTSTRAP );

    repl( show_prompt );

    debug_print( L"Freeing oblist\n", DEBUG_BOOTSTRAP );
    dec_ref( oblist );
    debug_dump_object( oblist, DEBUG_BOOTSTRAP );
    if ( dump_at_end ) {
        dump_pages( file_to_url_file( stdout ) );
    }

    summarise_allocation();
    curl_global_cleanup(  );
    return ( 0 );
}
