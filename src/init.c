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

#include "arith/peano.h"
#include "arith/ratio.h"
#include "debug.h"
#include "io/fopen.h"
#include "io/io.h"
#include "io/print.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/hashmap.h"
#include "memory/stack.h"
#include "ops/intern.h"
#include "ops/lispops.h"
#include "ops/meta.h"
#include "repl.h"
#include "time/psse_time.h"
#include "version.h"

/**
 * @brief If `pointer` is an exception, display that exception to stderr, 
 * decrement that exception, and return NIL; else return the pointer.
 * 
 * @param pointer a cons pointer.
 * @param location_descriptor a description of where the pointer was caught.
 * @return struct cons_pointer 
 */
struct cons_pointer check_exception( struct cons_pointer pointer,
                                     char *location_descriptor ) {
    struct cons_pointer result = NIL;

    struct cons_space_object *object = &pointer2cell( pointer );

    if ( exceptionp( pointer ) ) {
        fprintf( stderr, "ERROR: Exception at %s: ", location_descriptor );
        URL_FILE *ustderr = file_to_url_file( stderr );
        fwide( stderr, 1 );
        print( ustderr, object->payload.exception.payload );
        free( ustderr );

        dec_ref( pointer );
    } else {
        result = pointer;
    }

    return result;
}

struct cons_pointer init_documentation_symbol = NIL;
struct cons_pointer init_name_symbol = NIL;
struct cons_pointer init_primitive_symbol = NIL;

void maybe_bind_init_symbols(  ) {
    if ( nilp( init_documentation_symbol ) ) {
        init_documentation_symbol =
            c_string_to_lisp_keyword( L"documentation" );
    }
    if ( nilp( init_name_symbol ) ) {
        init_name_symbol = c_string_to_lisp_keyword( L"name" );
    }
    if ( nilp( init_primitive_symbol ) ) {
        init_primitive_symbol = c_string_to_lisp_keyword( L"primitive" );
    }
    if ( nilp( privileged_symbol_nil ) ) {
        privileged_symbol_nil = c_string_to_lisp_symbol( L"nil" );
    }
    // we can't make this string when we need it, because memory is then 
    // exhausted!
    if ( nilp( privileged_string_memory_exhausted ) ) {
        privileged_string_memory_exhausted =
            c_string_to_lisp_string( L"Memory exhausted." );
    }
    if ( nilp( privileged_keyword_location ) ) {
        privileged_keyword_location = c_string_to_lisp_keyword( L"location" );
    }
    if ( nilp( privileged_keyword_payload ) ) {
        privileged_keyword_location = c_string_to_lisp_keyword( L"payload" );
    }
}

void free_init_symbols(  ) {
    dec_ref( init_documentation_symbol );
    dec_ref( init_name_symbol );
    dec_ref( init_primitive_symbol );
}

/**
 * Bind this compiled `executable` function, as a Lisp function, to
 * this name in the `oblist`.
 * \todo where a function is not compiled from source, we could cache
 * the name on the source pointer. Would make stack frames potentially
 * more readable and aid debugging generally.
 */
struct cons_pointer bind_function( wchar_t *name,
                                   wchar_t *doc,
                                   struct cons_pointer ( *executable )
                                    ( struct stack_frame *,
                                      struct cons_pointer,
                                      struct cons_pointer ) ) {
    struct cons_pointer n = c_string_to_lisp_symbol( name );
    struct cons_pointer d = c_string_to_lisp_string( doc );

    struct cons_pointer meta =
        make_cons( make_cons( init_primitive_symbol, TRUE ),
                   make_cons( make_cons( init_name_symbol, n ),
                              make_cons( make_cons
                                         ( init_documentation_symbol, d ),
                                         NIL ) ) );

    struct cons_pointer r =
        check_exception( deep_bind( n, make_function( meta, executable ) ),
                         "bind_function" );

    dec_ref( n );
    dec_ref( d );

    return r;
}

/**
 * Bind this compiled `executable` function, as a Lisp special form, to
 * this `name` in the `oblist`.
 */
struct cons_pointer bind_special( wchar_t *name,
                                  wchar_t *doc,
                                  struct cons_pointer ( *executable )
                                   ( struct stack_frame *, struct cons_pointer,
                                     struct cons_pointer ) ) {
    struct cons_pointer n = c_string_to_lisp_symbol( name );
    struct cons_pointer d = c_string_to_lisp_string( doc );

    struct cons_pointer meta =
        make_cons( make_cons( init_primitive_symbol, TRUE ),
                   make_cons( make_cons( init_name_symbol, n ),
                              make_cons( make_cons
                                         ( init_documentation_symbol, d ),
                                         NIL ) ) );

    struct cons_pointer r =
        check_exception( deep_bind( n, make_special( meta, executable ) ),
                         "bind_special" );

    dec_ref( n );
    dec_ref( d );

    return r;
}

/**
 * Bind this `value` to this `symbol` in the `oblist`.
 */
struct cons_pointer
bind_symbol_value( struct cons_pointer symbol, struct cons_pointer value,
                   bool lock ) {
    struct cons_pointer r = check_exception( deep_bind( symbol, value ),
                                             "bind_symbol_value" );

    if ( lock && !exceptionp( r ) ) {
        struct cons_space_object *cell = &pointer2cell( r );

        cell->count = UINT32_MAX;
    }

    return r;
}

/**
 * Bind this `value` to this `name` in the `oblist`.
 */
struct cons_pointer bind_value( wchar_t *name, struct cons_pointer value,
                                bool lock ) {
    struct cons_pointer p = c_string_to_lisp_symbol( name );

    struct cons_pointer r = bind_symbol_value( p, value, lock );

    dec_ref( p );

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
void print_options( FILE *stream ) {
    fwprintf( stream, L"Expected options are:\n" );
    fwprintf( stream,
              L"\t-d\tDump memory to standard out at end of run (copious!);\n" );
    fwprintf( stream, L"\t-h\tPrint this message and exit;\n" );
    fwprintf( stream, L"\t-p\tShow a prompt (default is no prompt);\n" );
#ifdef DEBUG
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
#endif
}

/**
 * main entry point; parse command line arguments, initialise the environment,
 * and enter the read-eval-print loop.
 */
int main( int argc, char *argv[] ) {
    int option;
    bool dump_at_end = false;
    bool show_prompt = false;
    char *infilename = NULL;

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
            case 'i':
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

    initialise_cons_pages(  );

    maybe_bind_init_symbols(  );


    if ( show_prompt ) {
        print_banner(  );
    }

    debug_print( L"About to initialise oblist\n", DEBUG_BOOTSTRAP );

    oblist = make_hashmap( 32, NIL, TRUE );

    debug_print( L"About to bind\n", DEBUG_BOOTSTRAP );

    /*
     * privileged variables (keywords)
     */
    bind_symbol_value( privileged_symbol_nil, NIL, true );
    bind_value( L"t", TRUE, true );
    bind_symbol_value( privileged_keyword_location, TRUE, true );
    bind_symbol_value( privileged_keyword_payload, TRUE, true );

    /*
     * standard input, output, error and sink streams
     * attempt to set wide character acceptance on all streams
     */
    URL_FILE *sink = url_fopen( "/dev/null", "w" );
    fwide( stdin, 1 );
    fwide( stdout, 1 );
    fwide( stderr, 1 );
    fwide( sink->handle.file, 1 );

    FILE *infile = infilename == NULL ? stdin : fopen( infilename, "r" );


    lisp_io_in =
        bind_value( C_IO_IN,
                    make_read_stream( file_to_url_file( infile ),
                                      make_cons( make_cons
                                                 ( c_string_to_lisp_keyword
                                                   ( L"url" ),
                                                   c_string_to_lisp_string
                                                   ( L"system:standard input" ) ),
                                                 NIL ) ), false );
    lisp_io_out =
        bind_value( C_IO_OUT,
                    make_write_stream( file_to_url_file( stdout ),
                                       make_cons( make_cons
                                                  ( c_string_to_lisp_keyword
                                                    ( L"url" ),
                                                    c_string_to_lisp_string
                                                    ( L"system:standard output]" ) ),
                                                  NIL ) ), false );
    bind_value( L"*log*",
                make_write_stream( file_to_url_file( stderr ),
                                   make_cons( make_cons
                                              ( c_string_to_lisp_keyword
                                                ( L"url" ),
                                                c_string_to_lisp_string
                                                ( L"system:standard log" ) ),
                                              NIL ) ), false );
    bind_value( L"*sink*",
                make_write_stream( sink,
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
                              show_prompt ? c_string_to_lisp_symbol( L":: " ) :
                              NIL, false );
    /*
     * primitive function operations
     */
    /* TODO: docstrings should be moved to a header file, or even to an at-run-time resolution system. 
     * HTTP from an address at journeyman? */
    bind_function( L"absolute",
                   L"`(absolute arg)`: If `arg` is a number, return the absolute value of that number, else `nil`.",
                   &lisp_absolute );
    bind_function( L"add",
                   L"`(+ args...)`: If `args` are all numbers, return the sum of those numbers.",
                   &lisp_add );
    bind_function( L"and",
                   L"`(and args...)`: Return a logical `and` of all the arguments and return `t` only if all are truthy, else `nil`.",
                   &lisp_and );
    bind_function( L"append",
                   L"`(append args...)`: If args are all collections, return the concatenation of those collections.",
                   &lisp_append );
    bind_function( L"apply",
                   L"`(apply f args)`: If `f` is usable as a function, and `args` is a collection, apply `f` to `args` and return the value.",
                   &lisp_apply );
    bind_function( L"assoc",
                   L"`(assoc key store)`: Return the value associated with this `key` in this `store`.",
                   &lisp_assoc );
    bind_function( L"car",
                   L"`(car arg)`: If `arg` is a sequence, return the item which is the head of that sequence.",
                   &lisp_car );
    bind_function( L"cdr",
                   L"`(cdr arg)`: If `arg` is a sequence, return the remainder of that sequence with the first item removed.",
                   &lisp_cdr );
    bind_function( L"close",
                   L"`(close stream)`: If `stream` is a stream, close that stream.",
                   &lisp_close );
    bind_function( L"cons",
                   L"`(cons a b)`: Return a cons cell whose `car` is `a` and whose `cdr` is `b`.",
                   &lisp_cons );
    bind_function( L"count",
                   L"`(count s)`: Return the number of items in the sequence `s`.",
                   &lisp_count );
    bind_function( L"divide",
                   L"`(/ a b)`: If `a` and `b` are both numbers, return the numeric result of dividing `a` by `b`.",
                   &lisp_divide );
    bind_function( L"eq?",
                   L"`(eq? args...)`: Return `t` if all args are the exact same object, else `nil`.",
                   &lisp_eq );
    bind_function( L"equal?",
                   L"`(equal? args...)`: Return `t` if all args have logically equivalent value, else `nil`.",
                   &lisp_equal );
    bind_function( L"eval", L"", &lisp_eval );
    bind_function( L"exception",
                   L"`(exception message)`: Return (throw) an exception with this `message`.",
                   &lisp_exception );
    bind_function( L"get-hash",
                   L"`(get-hash arg)`: returns the natural number hash value of `arg`.",
                   &lisp_get_hash );
    bind_function( L"hashmap",
                   L"`(hashmap n-buckets hashfn store acl)`: Return a new hashmap, with `n-buckets` buckets and this `hashfn`, containing the content of this `store`.",
                   lisp_make_hashmap );
    bind_function( L"inspect",
                   L"`(inspect object ouput-stream)`: Print details of this `object` to this `output-stream` or `*out*`.",
                   &lisp_inspect );
    bind_function( L"interned?",
                   L"`(interned? key store)`: Return `t` if the symbol or keyword `key` is bound in this `store`, else `nil`.",
                   &lisp_internedp );
    bind_function( L"keys",
                   L"`(keys store)`: Return a list of all keys in this `store`.",
                   &lisp_keys );
    bind_function( L"list",
                   L"`(list args...)`: Return a list of these `args`.",
                   &lisp_list );
    bind_function( L"mapcar",
                   L"`(mapcar function sequence)`: Apply `function` to each element of `sequence` in turn, and return a sequence of the results.",
                   &lisp_mapcar );
    bind_function( L"meta",
                   L"`(meta symbol)`: If the binding of `symbol` has metadata, return that metadata, else `nil`.",
                   &lisp_metadata );
    bind_function( L"metadata",
                   L"`(metadata symbol)`: If the binding of `symbol` has metadata, return that metadata, else `nil`.",
                   &lisp_metadata );
    bind_function( L"multiply",
                   L"`(* args...)` Multiply these `args`, all of which should be numbers.",
                   &lisp_multiply );
    bind_function( L"negative?",
                   L"`(negative? n)`: Return `t` if `n` is a negative number, else `nil`.",
                   &lisp_is_negative );
    bind_function( L"not",
                   L"`(not arg)`: Return`t` only if `arg` is `nil`, else `nil`.",
                   &lisp_not );
    bind_function( L"oblist",
                   L"`(oblist)`: Return the current symbol bindings, as a map.",
                   &lisp_oblist );
    bind_function( L"open",
                   L"`(open url write?)`: Open a stream to this `url`. If `write?` is present and is non-nil, open it for writing, else reading.",
                   &lisp_open );
    bind_function( L"or",
                   L"`(or args...)`: Return a logical `or` of all the arguments and return `t` if any is truthy, else `nil`.",
                   &lisp_or );
    bind_function( L"print",
                   L"`(print object stream)`: Print `object` to `stream`, if specified, else to `*out*`.",
                   &lisp_print );
    bind_function( L"println",
                   L"`(println stream)`: Print a new line character to `stream`, if specified, else to `*out*`.",
                   &lisp_print );
    bind_function( L"put!", L"", lisp_hashmap_put );
    bind_function( L"put-all!",
                   L"`(put-all! dest source)`: If `dest` is a namespace and is writable, copies all key-value pairs from `source` into `dest`.",
                   &lisp_hashmap_put_all );
    bind_function( L"ratio->real",
                   L"`(ratio->real r)`: If `r` is a rational number, return the real number equivalent.",
                   &lisp_ratio_to_real );
    bind_function( L"read",
                   L"`(read stream)`: read one complete lisp form and return it. If `stream` is specified and is a read stream, then read from that stream, else the stream which is the value of  `*in*` in the environment.",
                   &lisp_read );
    bind_function( L"read-char",
                   L"`(read-char stream)`: Return the next character. If `stream` is specified and is a read stream, then read from that stream, else the stream which is the value of  `*in*` in the environment.",
                   &lisp_read_char );
    bind_function( L"repl",
                   L"`(repl prompt input output)`: Starts a new read-eval-print-loop. All arguments are optional.",
                   &lisp_repl );
    bind_function( L"reverse",
                   L"`(reverse sequence)` Returns a sequence of the top level elements of this `sequence`, which may be a list or a string, in the reverse order.",
                   &lisp_reverse );
    bind_function( L"set", L"", &lisp_set );
    bind_function( L"slurp",
                   L"`(slurp read-stream)` Read all the characters from `read-stream` to the end of stream, and return them as a string.",
                   &lisp_slurp );
    bind_function( L"source",
                   L"`(source  object)`: If `object` is an interpreted function or interpreted special form, returns the source code; else nil.",
                   &lisp_source );
    bind_function( L"subtract",
                   L"`(- a b)`: Subtracts `b` from `a` and returns the result. Expects both arguments to be numbers.",
                   &lisp_subtract );
    bind_function( L"throw", L"", &lisp_exception );
    bind_function( L"time",
                   L"`(time arg)`: Return a time object. If an `arg` is supplied, it should be an integer which will be interpreted as a number of microseconds since the big bang, which is assumed to have happened 441,806,400,000,000,000 seconds before the UNIX epoch.",
                   &lisp_time );
    bind_function( L"type",
                   L"`(type object)`: returns the type of the specified `object`. Currently (0.0.6) the type is returned as a four character string; this may change.",
                   &lisp_type );
    bind_function( L"+",
                   L"`(+ args...)`: If `args` are all numbers, return the sum of those numbers.",
                   &lisp_add );
    bind_function( L"*",
                   L"`(* args...)` Multiply these `args`, all of which should be numbers.",
                   &lisp_multiply );
    bind_function( L"-",
                   L"`(- a b)`: Subtracts `b` from `a` and returns the result. Expects both arguments to be numbers.",
                   &lisp_subtract );
    bind_function( L"/",
                   L"`(/ a b)`: If `a` and `b` are both numbers, return the numeric result of dividing `a` by `b`.",
                   &lisp_divide );
    bind_function( L"=",
                   L"`(equal? args...)`: Return `t` if all args have logically equivalent value, else `nil`.",
                   &lisp_equal );
    /*
     * primitive special forms
     */
    bind_special( L"cond",
                  L"`(cond clauses...)`: Conditional evaluation, `clauses` is a sequence of lists of forms such that if evaluating the first form in any clause returns non-`nil`, the subsequent forms in that clause will be evaluated and the value of the last returned; but any subsequent clauses will not be evaluated.",
                  &lisp_cond );
    bind_special( L"lambda",
                  L"`(lambda arg-list forms...)`: Construct an interpretable λ funtion.",
                  &lisp_lambda );
    bind_special( L"\u03bb", L"", &lisp_lambda ); // λ
    bind_special( L"let",
                  L"`(let bindings forms)`: Bind these `bindings`, which should be specified as an association list, into the local environment and evaluate these forms sequentially in that context, returning the value of the last.",
                  &lisp_let );
    bind_special( L"nlambda",
                  L"`(nlamda arg-list forms...)`: Construct an interpretable special form. When the form is interpreted, arguments specified in the `arg-list` will not be evaluated.",
                  &lisp_nlambda );
    bind_special( L"n\u03bb", L"`(nlamda arg-list forms...)`: Construct an interpretable special form. When the form is interpreted, arguments specified in the `arg-list` will not be evaluated.", &lisp_nlambda );  // nλ
    bind_special( L"progn",
                  L"`(progn forms...)` Evaluate `forms` sequentially, and return the value of the last.",
                  &lisp_progn );
    bind_special( L"quote",
                  L"`(quote form)`: Returns `form`, unevaluated. More idiomatically expressed `'form`, where the quote mark is a reader macro which is expanded to `(quote form)`.",
                  &lisp_quote );
    bind_special( L"set!",
                  L"`(set! symbol value namespace)`: Binds `symbol` in  `namespace` to the value of `value`, altering the namespace in so doing, and returns `value`. If `namespace` is not specified, it defaults to the default namespace.",
                  &lisp_set_shriek );
    bind_special( L"try", L"", &lisp_try );
    debug_print( L"Initialised oblist\n", DEBUG_BOOTSTRAP );
    debug_dump_object( oblist, DEBUG_BOOTSTRAP );

    repl( show_prompt );

    debug_dump_object( oblist, DEBUG_BOOTSTRAP );

    debug_print( L"Freeing oblist\n", DEBUG_BOOTSTRAP );
    while ( ( pointer2cell( oblist ) ).count > 0 ) {
        fprintf( stderr, "Dangling refs on oblist: %d\n",
                 ( pointer2cell( oblist ) ).count );
        dec_ref( oblist );
    }

    free_init_symbols(  );

    if ( dump_at_end ) {
        dump_pages( file_to_url_file( stdout ) );
    }

    summarise_allocation(  );
    curl_global_cleanup(  );
    return ( 0 );
}
