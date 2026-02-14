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

#include <getopt.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

/* libcurl, used for io */
#include <curl/curl.h>

#include "version.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/stack.h"
#include "debug.h"
#include "memory/hashmap.h"
#include "ops/intern.h"
#include "io/io.h"
#include "io/fopen.h"
#include "ops/lispops.h"
#include "ops/meta.h"
#include "arith/peano.h"
#include "io/print.h"
#include "repl.h"
#include "io/fopen.h"
#include "time/psse_time.h"

/**
 * @brief If `pointer` is an exception, display that exception to stderr, 
 * decrement that exception, and return NIL; else return the pointer.
 * 
 * @param pointer a cons pointer.
 * @param location_descriptor a description of where the pointer was caught.
 * @return struct cons_pointer 
 */
struct cons_pointer check_exception( struct cons_pointer pointer, char * location_descriptor) {
    struct cons_pointer result = NIL;

    struct cons_space_object * object = &pointer2cell( pointer);

    if ( exceptionp( pointer)) {
        fprintf( stderr, "ERROR: Exception at %s: ", location_descriptor);
        URL_FILE *ustderr = file_to_url_file( stderr );
        fwide( stderr, 1 );
        print( ustderr, object->payload.exception.payload );
        free( ustderr );

        dec_ref( pointer);
    } else {
        result = pointer;
    }

    return result;
}

struct cons_pointer init_name_symbol = NIL;
struct cons_pointer init_primitive_symbol = NIL;

void maybe_bind_init_symbols() {
    if ( nilp( init_name_symbol)) {
        init_name_symbol = c_string_to_lisp_keyword( L"name" );
    }
    if ( nilp( init_primitive_symbol)) {
        init_primitive_symbol = c_string_to_lisp_keyword( L"primitive" );
    }
    if ( nilp( privileged_symbol_nil)) {
        privileged_symbol_nil = c_string_to_lisp_symbol( L"nil");
    }
}

void free_init_symbols() {
    dec_ref( init_name_symbol);
    dec_ref( init_primitive_symbol);
}

/**
 * Bind this compiled `executable` function, as a Lisp function, to
 * this name in the `oblist`.
 * \todo where a function is not compiled from source, we could cache
 * the name on the source pointer. Would make stack frames potentially
 * more readable and aid debugging generally.
 */
struct cons_pointer bind_function( wchar_t *name, struct cons_pointer ( *executable )
                     ( struct stack_frame *,
                       struct cons_pointer, struct cons_pointer ) ) {
    struct cons_pointer n = c_string_to_lisp_symbol( name );
    struct cons_pointer meta =
        make_cons( make_cons( init_primitive_symbol, TRUE ),
                   make_cons( make_cons( init_name_symbol, n ),
                              NIL ) );

    struct cons_pointer r = check_exception( 
        deep_bind( n, make_function( meta, executable ) ),
                    "bind_function");
    
    dec_ref( n);

    return r;
}

/**
 * Bind this compiled `executable` function, as a Lisp special form, to
 * this `name` in the `oblist`.
 */
struct cons_pointer bind_special( wchar_t *name, struct cons_pointer ( *executable )
                    ( struct stack_frame *,
                      struct cons_pointer, struct cons_pointer ) ) {
    struct cons_pointer n = c_string_to_lisp_symbol( name );

    struct cons_pointer meta =
        make_cons( make_cons( init_primitive_symbol, TRUE ),
                   make_cons( make_cons( init_name_symbol, n), NIL ) );

    struct cons_pointer r = 
        check_exception(deep_bind( n, make_special( meta, executable ) ),
                    "bind_special");
    
    dec_ref( n);

    return r;
}

/**
 * Bind this `value` to this `symbol` in the `oblist`.
 */
struct cons_pointer 
bind_symbol_value( struct cons_pointer symbol, struct cons_pointer value, bool lock) {
    struct cons_pointer r = check_exception( 
        deep_bind( symbol, value ),
            "bind_symbol_value");

    if ( lock && !exceptionp( r)){
        struct cons_space_object * cell = & pointer2cell( r);

        cell->count = UINT32_MAX;
    }

    return r;
}

/**
 * Bind this `value` to this `name` in the `oblist`.
 */
struct cons_pointer bind_value( wchar_t *name, struct cons_pointer value, bool lock ) {
    struct cons_pointer p = c_string_to_lisp_symbol( name );

    struct cons_pointer r = bind_symbol_value( p, value, lock);

    dec_ref( p);

    return r;
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
    char * infilename = NULL;

    setlocale( LC_ALL, "" );
    if ( io_init(  ) != 0 ) {
        fputs( "Failed to initialise I/O subsystem\n", stderr );
        exit( 1 );
    }

    while ( ( option = getopt( argc, argv, "phdv:i:" ) ) != -1 ) {
        switch ( option ) {
            case 'd':
                dump_at_end = true;
                break;
            case 'h':
                print_banner(  );
                print_options( stdout );
                exit( 0 );
                break;
            case 'i' :
                infilename = optarg;
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

    initialise_cons_pages();

    maybe_bind_init_symbols();


    if ( show_prompt ) {
        print_banner(  );
    }

    debug_print( L"About to initialise oblist\n", DEBUG_BOOTSTRAP );

    oblist = make_hashmap( 32, NIL, TRUE );

    debug_print( L"About to bind\n", DEBUG_BOOTSTRAP );

    /*
     * privileged variables (keywords)
     */
    bind_symbol_value( privileged_symbol_nil, NIL, true);
    bind_value( L"t", TRUE, true );

    /*
     * standard input, output, error and sink streams
     * attempt to set wide character acceptance on all streams
     */
    URL_FILE *sink = url_fopen( "/dev/null", "w" );
    fwide( stdin, 1 );
    fwide( stdout, 1 );
    fwide( stderr, 1 );
    fwide( sink->handle.file, 1 );

    FILE *infile = infilename == NULL ? stdin : fopen( infilename, "r");


    lisp_io_in = bind_value( C_IO_IN, make_read_stream(  file_to_url_file(infile),
                                    make_cons( make_cons
                                                ( c_string_to_lisp_keyword
                                                ( L"url" ),
                                                c_string_to_lisp_string
                                                ( L"system:standard input" ) ),
                                                NIL ) ), false );
    lisp_io_out = bind_value( C_IO_OUT,
                make_write_stream( file_to_url_file( stdout ),
                                   make_cons( make_cons
                                              ( c_string_to_lisp_keyword
                                                ( L"url" ),
                                                c_string_to_lisp_string
                                                ( L"system:standard output]" ) ),
                                              NIL ) ), false);
    bind_value( L"*log*", make_write_stream( file_to_url_file( stderr ),
                                             make_cons( make_cons
                                                        ( c_string_to_lisp_keyword
                                                          ( L"url" ),
                                                          c_string_to_lisp_string
                                                          ( L"system:standard log" ) ),
                                                        NIL ) ), false );
    bind_value( L"*sink*", make_write_stream( sink,
                                              make_cons( make_cons
                                                         ( c_string_to_lisp_keyword
                                                           ( L"url" ),
                                                           c_string_to_lisp_string
                                                           ( L"system:standard sink" ) ),
                                                         NIL ) ), false );
    /*
     * the default prompt
     */
    prompt_name = bind_value( L"*prompt*",
                show_prompt ? c_string_to_lisp_symbol( L":: " ) : NIL, false );
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
    bind_function( L"list", &lisp_list );
    bind_function( L"mapcar", &lisp_mapcar );
    bind_function( L"meta", &lisp_metadata );
    bind_function( L"metadata", &lisp_metadata );
    bind_function( L"multiply", &lisp_multiply );
    bind_function( L"negative?", &lisp_is_negative );
    bind_function( L"oblist", &lisp_oblist );
    bind_function( L"open", &lisp_open );
    bind_function( L"print", &lisp_print );
    bind_function( L"put!", lisp_hashmap_put );
    bind_function( L"put-all!", &lisp_hashmap_put_all );
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
    bind_special( L"let", &lisp_let );
    bind_special( L"nlambda", &lisp_nlambda );
    bind_special( L"n\u03bb", &lisp_nlambda );
    bind_special( L"progn", &lisp_progn );
    bind_special( L"quote", &lisp_quote );
    bind_special( L"set!", &lisp_set_shriek );
    bind_special( L"try", &lisp_try );
    debug_print( L"Initialised oblist\n", DEBUG_BOOTSTRAP );
    debug_dump_object( oblist, DEBUG_BOOTSTRAP );

    repl( show_prompt );

    debug_dump_object( oblist, DEBUG_BOOTSTRAP );
    if ( dump_at_end ) {
        dump_pages( file_to_url_file( stdout ) );
    }

    debug_print( L"Freeing oblist\n", DEBUG_BOOTSTRAP );
    dec_ref( oblist );
    free_init_symbols();

    summarise_allocation(  );
    curl_global_cleanup(  );
    return ( 0 );
}
