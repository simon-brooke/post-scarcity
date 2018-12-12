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

int print_use_colours = 0;

void print_string_contents( FILE * output, struct cons_pointer pointer ) {
    if ( stringp( pointer ) || symbolp( pointer ) ) {
        struct cons_space_object *cell = &pointer2cell( pointer );
        wint_t c = cell->payload.string.character;

        if ( c != '\0' ) {
            fputwc( c, output );
        }
        print_string_contents( output, cell->payload.string.cdr );
    }
}

void print_string( FILE * output, struct cons_pointer pointer ) {
    fputwc( btowc( '"' ), output );
    print_string_contents( output, pointer );
    fputwc( btowc( '"' ), output );
}

/**
 * Print a single list cell (cons cell).
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
            fputws( L"\x1B[39m", output );
            break;
        case INTEGERTV:
            if ( print_use_colours ) {
                fputws( L"\x1B[34m", output );
            }
            fwprintf( output, L"%ld%", cell.payload.integer.value );
            if ( print_use_colours ) {
                fputws( L"\x1B[39m", output );
            }
            break;
        case LAMBDATV:
            fwprintf( output, L"lambda" /* "λ" */  );
            break;
        case NILTV:
            fwprintf( output, L"nil" );
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
            if ( print_use_colours ) {
                fputws( L"\x1B[39m", output );
            }
            free( buffer );
            break;
        case STRINGTV:
            if ( print_use_colours ) {
                fputws( L"\x1B[36m", output );
            }
            print_string( output, pointer );
            if ( print_use_colours ) {
                fputws( L"\x1B[39m", output );
            }
            break;
        case SYMBOLTV:
            if ( print_use_colours )
                fputws( L"\x1B[1;33m", output );
            print_string_contents( output, pointer );
            if ( print_use_colours )
                fputws( L"\x1B[0;39m", output );
            break;
        case TRUETV:
            fwprintf( output, L"t" );
            break;
        case FUNCTIONTV:
            fwprintf( output, L"(Function)" );
            break;
        case SPECIALTV:
            fwprintf( output, L"(Special form)" );
            break;
        default:
            fwprintf( stderr,
                      L"%sError: Unrecognised tag value %d (%c%c%c%c)%s\n",
                      "\x1B[31m",
                      cell.tag.value, cell.tag.bytes[0], cell.tag.bytes[1],
                      cell.tag.bytes[2], cell.tag.bytes[3], "\x1B[39m" );
            break;
    }
}
