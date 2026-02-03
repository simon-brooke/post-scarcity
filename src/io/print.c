/*
 * print.c
 *
 * First pass at a printer, for bootstrapping.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/hashmap.h"
#include "arith/integer.h"
#include "ops/intern.h"
#include "memory/stack.h"
#include "io/print.h"
#include "time/psse_time.h"
#include "memory/vectorspace.h"

/**
 * print all the characters in the symbol or string indicated by `pointer`
 * onto this `output`; if `pointer` does not indicate a string or symbol,
 * don't print anything but just return.
 */
void print_string_contents( URL_FILE * output, struct cons_pointer pointer ) {
    while ( stringp( pointer ) || symbolp( pointer ) || keywordp( pointer ) ) {
        struct cons_space_object *cell = &pointer2cell( pointer );
        wchar_t c = cell->payload.string.character;

        if ( c != '\0' ) {
            url_fputwc( c, output );
        }
        pointer = cell->payload.string.cdr;
    }
}

/**
 * print all the characters in the string indicated by `pointer` onto
 * the stream at this `output`, prepending and appending double quote
 * characters.
 */
void print_string( URL_FILE * output, struct cons_pointer pointer ) {
    url_fputwc( btowc( '"' ), output );
    print_string_contents( output, pointer );
    url_fputwc( btowc( '"' ), output );
}

/**
 * Print a single list cell (cons cell) indicated by `pointer` to the
 * stream indicated by `output`. if `initial_space` is `true`, prepend
 * a space character.
 */
void
print_list_contents( URL_FILE * output, struct cons_pointer pointer,
                     bool initial_space ) {
    struct cons_space_object *cell = &pointer2cell( pointer );

    switch ( cell->tag.value ) {
        case CONSTV:
            if ( initial_space ) {
                url_fputwc( btowc( ' ' ), output );
            }
            print( output, cell->payload.cons.car );

            print_list_contents( output, cell->payload.cons.cdr, true );
            break;
        case NILTV:
            break;
        default:
            url_fwprintf( output, L" . " );
            print( output, pointer );
    }
}

void print_list( URL_FILE * output, struct cons_pointer pointer ) {
    url_fputws( L"(", output );
    print_list_contents( output, pointer, false );
    url_fputws( L")", output );
}

void print_map( URL_FILE * output, struct cons_pointer map ) {
    if ( hashmapp( map ) ) {
        struct vector_space_object *vso = pointer_to_vso( map );

        url_fputwc( btowc( '{' ), output );

        for ( struct cons_pointer ks = hashmap_keys( map ); !nilp( ks );
              ks = c_cdr( ks ) ) {
            struct cons_pointer key = c_car( ks );
            print( output, key );
            url_fputwc( btowc( ' ' ), output );
            print( output, hashmap_get( map, key ) );

            if ( !nilp( c_cdr( ks ) ) ) {
                url_fputws( L", ", output );
            }
        }

        url_fputwc( btowc( '}' ), output );
    }
}

void print_vso( URL_FILE * output, struct cons_pointer pointer ) {
    struct vector_space_object *vso = pointer_to_vso( pointer );
    switch ( vso->header.tag.value ) {
        case HASHTV:
            print_map( output, pointer );
            break;
            // \todo: others.
        default:
            fwprintf( stderr, L"Unrecognised vector-space type '%d'\n",
                      vso->header.tag.value );
    }
}

/**
 * stolen from https://stackoverflow.com/questions/11656241/how-to-print-uint128-t-number-using-gcc
 */
void print_128bit( URL_FILE * output, __int128_t n ) {
    if ( n == 0 ) {
        fwprintf( stderr, L"0" );
    } else {
        char str[40] = { 0 };   // log10(1 << 128) + '\0'
        char *s = str + sizeof( str ) - 1;  // start at the end
        while ( n != 0 ) {
            if ( s == str )
                return;         // never happens

            *--s = "0123456789"[n % 10];  // save last digit
            n /= 10;            // drop it
        }
        url_fwprintf( output, L"%s", s );
    }
}


/**
 * Print the cons-space object indicated by `pointer` to the stream indicated
 * by `output`.
 */
struct cons_pointer print( URL_FILE * output, struct cons_pointer pointer ) {
    struct cons_space_object cell = pointer2cell( pointer );
    char *buffer;

    /*
     * Because tags have values as well as bytes, this if ... else if
     * statement can ultimately be replaced by a switch, which will be neater.
     */
    switch ( cell.tag.value ) {
        case CONSTV:
            print_list( output, pointer );
            break;
        case EXCEPTIONTV:
            url_fputws( L"\nException: ", output );
            dump_stack_trace( output, pointer );
            break;
        case FUNCTIONTV:
            url_fputws( L"<Function: ", output );
            print( output, cell.payload.function.meta );
            url_fputwc( L'>', output );
            break;
        case INTEGERTV:{
                struct cons_pointer s = integer_to_string( pointer, 10 );
                inc_ref( s );
                print_string_contents( output, s );
                dec_ref( s );
            }
            break;
        case KEYTV:
            url_fputws( L":", output );
            print_string_contents( output, pointer );
            break;
        case LAMBDATV:{
                url_fputws( L"<Anonymous Function: ", output );
                struct cons_pointer to_print =
                    make_cons( c_string_to_lisp_symbol( L"\u03bb" ),
                               make_cons( cell.payload.lambda.args,
                                          cell.payload.lambda.body ) );
                inc_ref( to_print );

                print( output, to_print );

                dec_ref( to_print );
                url_fputwc( L'>', output );
            }
            break;
        case NILTV:
            url_fwprintf( output, L"nil" );
            break;
        case NLAMBDATV:{
                url_fputws( L"<Anonymous Special Form: ", output );
                struct cons_pointer to_print =
                    make_cons( c_string_to_lisp_symbol( L"n\u03bb" ),
                               make_cons( cell.payload.lambda.args,
                                          cell.payload.lambda.body ) );
                inc_ref( to_print );

                print( output, to_print );

                dec_ref( to_print );
                url_fputwc( L'>', output );
            }
            break;
        case RATIOTV:
            print( output, cell.payload.ratio.dividend );
            url_fputws( L"/", output );
            print( output, cell.payload.ratio.divisor );
            break;
        case READTV:
            url_fwprintf( output, L"<Input stream: " );
            print( output, cell.payload.stream.meta );
            url_fputwc( L'>', output );
            break;
        case REALTV:
            /* \todo using the C heap is a bad plan because it will fragment.
             * As soon as I have working vector space I'll use a special purpose
             * vector space object */
            buffer = ( char * ) malloc( 24 );
            memset( buffer, 0, 24 );
            /* format it really long, then clear the trailing zeros */
            sprintf( buffer, "%-.23Lg", cell.payload.real.value );
            if ( strchr( buffer, '.' ) != NULL ) {
                for ( int i = strlen( buffer ) - 1; buffer[i] == '0'; i-- ) {
                    buffer[i] = '\0';
                }
            }
            url_fwprintf( output, L"%s", buffer );
            free( buffer );
            break;
        case STRINGTV:
            print_string( output, pointer );
            break;
        case SYMBOLTV:
            print_string_contents( output, pointer );
            break;
        case SPECIALTV:
            url_fwprintf( output, L"<Special form: " );
            print( output, cell.payload.special.meta );
            url_fputwc( L'>', output );
            break;
        case TIMETV:
            url_fwprintf( output, L"<Time: " );
            print_string( output, time_to_string( pointer ) );
            url_fputws( L"; ", output );
            print_128bit( output, pointer2cell( pointer ).payload.time.value );
            url_fputwc( L'>', output );
            break;
        case TRUETV:
            url_fwprintf( output, L"t" );
            break;
        case VECTORPOINTTV:
            print_vso( output, pointer );
            break;
        case WRITETV:
            url_fwprintf( output, L"<Output stream: " );
            print( output, cell.payload.stream.meta );
            url_fputwc( L'>', output );
            break;
        default:
            fwprintf( stderr,
                      L"Error: Unrecognised tag value %d (%4.4s)\n",
                      cell.tag.value, &cell.tag.bytes[0] );
            break;
    }

    return pointer;
}

void println( URL_FILE * output ) {
    url_fputws( L"\n", output );
}
