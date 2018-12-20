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

#include "conspage.h"
#include "consspaceobject.h"
#include "integer.h"
#include "print.h"

/**
 * Whether or not we colorise output.
 * TODO: this should be a Lisp symbol binding, not a C variable.
 */
int print_use_colours = 0;

/**
 * print all the characters in the symbol or string indicated by `pointer`
 * onto this `output`; if `pointer` does not indicate a string or symbol,
 * don't print anything but just return.
 */
void print_string_contents( FILE * output, struct cons_pointer pointer ) {
    while ( stringp( pointer ) || symbolp( pointer ) ) {
        struct cons_space_object *cell = &pointer2cell( pointer );
        wint_t c = cell->payload.string.character;

        if ( c != '\0' ) {
            fputwc( c, output );
        }
        pointer = cell->payload.string.cdr;
    }
}

/**
 * print all the characters in the string indicated by `pointer` onto
 * the stream at this `output`, prepending and appending double quote
 * characters.
 */
void print_string( FILE * output, struct cons_pointer pointer ) {
    fputwc( btowc( '"' ), output );
    print_string_contents( output, pointer );
    fputwc( btowc( '"' ), output );
}

/**
 * Print a single list cell (cons cell) indicated by `pointer` to the
 * stream indicated by `output`. if `initial_space` is `true`, prepend
 * a space character.
 */
void
print_list_contents( FILE * output, struct cons_pointer pointer,
                     bool initial_space ) {
    struct cons_space_object *cell = &pointer2cell( pointer );

    switch ( cell->tag.value ) {
        case CONSTV:
            if ( initial_space ) {
                fputwc( btowc( ' ' ), output );
            }
            print( output, cell->payload.cons.car );

            print_list_contents( output, cell->payload.cons.cdr, true );
            break;
        case NILTV:
            break;
        default:
            fwprintf( output, L" . " );
            print( output, pointer );
    }
}

void print_list( FILE * output, struct cons_pointer pointer ) {
    if ( print_use_colours ) {
        fwprintf( output, L"%s(%s", "\x1B[31m", "\x1B[39m" );
    } else {
        fputws( L"(", output );
    };

    print_list_contents( output, pointer, false );
    if ( print_use_colours ) {
        fwprintf( output, L"%s)%s", "\x1B[31m", "\x1B[39m" );
    } else {
        fputws( L")", output );
    }

}

/**
 * Print the cons-space object indicated by `pointer` to the stream indicated
 * by `output`.
 */
void print( FILE * output, struct cons_pointer pointer ) {
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
            fwprintf( output, L"\n%sException: ",
                      print_use_colours ? "\x1B[31m" : "" );
            print_string_contents( output, cell.payload.exception.message );
            break;
        case FUNCTIONTV:
            fwprintf( output, L"(Function)" );
            break;
        case INTEGERTV:
            if ( print_use_colours ) {
                fputws( L"\x1B[34m", output );
            }
            fwprintf( output, L"%ld%", cell.payload.integer.value );
            break;
        case LAMBDATV:
            print( output, make_cons( c_string_to_lisp_symbol( "lambda" ),
                                      make_cons( cell.payload.lambda.args,
                                                 cell.payload.lambda.
                                                 body ) ) );
            break;
        case NILTV:
            fwprintf( output, L"nil" );
            break;
        case NLAMBDATV:
            print( output, make_cons( c_string_to_lisp_symbol( "nlambda" ),
                                      make_cons( cell.payload.lambda.args,
                                                 cell.payload.lambda.
                                                 body ) ) );
            break;
        case READTV:
            fwprintf( output, L"(Input stream)" );
            break;
        case REALTV:
            /* TODO: using the C heap is a bad plan because it will fragment.
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
            if ( print_use_colours ) {
                fputws( L"\x1B[34m", output );
            }
            fwprintf( output, L"%s", buffer );
            free( buffer );
            break;
        case STRINGTV:
            if ( print_use_colours ) {
                fputws( L"\x1B[36m", output );
            }
            print_string( output, pointer );
            break;
        case SYMBOLTV:
            if ( print_use_colours ) {
                fputws( L"\x1B[1;33m", output );
            }
            print_string_contents( output, pointer );
            break;
        case SPECIALTV:
            fwprintf( output, L"(Special form)" );
            break;
        case TRUETV:
            fwprintf( output, L"t" );
            break;
        default:
            fwprintf( stderr,
                      L"%sError: Unrecognised tag value %d (%c%c%c%c)\n",
                      print_use_colours ? "\x1B[31m" : "",
                      cell.tag.value, cell.tag.bytes[0], cell.tag.bytes[1],
                      cell.tag.bytes[2], cell.tag.bytes[3] );
            break;
    }

    if ( print_use_colours ) {
        fputws( L"\x1B[39m", output );
    }
}
