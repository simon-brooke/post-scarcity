/**
 * print.c
 *
 * First pass at a printer, for bootstrapping.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdio.h>
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
 * Print a single list cell (cons cell). TODO: does not handle dotted pairs.
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
    fputwc( btowc( '(' ), output );
    print_list_contents( output, pointer, false );
    fputwc( btowc( ')' ), output );
}

void print( FILE * output, struct cons_pointer pointer ) {
    struct cons_space_object cell = pointer2cell( pointer );

    /*
     * Because tags have values as well as bytes, this if ... else if
     * statement can ultimately be replaced by a switch, which will be neater. 
     */
    switch ( cell.tag.value ) {
    case CONSTV:
        print_list( output, pointer );
        break;
    case INTEGERTV:
        fwprintf( output, L"%ld", cell.payload.integer.value );
        break;
    case NILTV:
        fwprintf( output, L"nil" );
        break;
    case REALTV:
        fwprintf( output, L"%Lf", cell.payload.real.value );
        break;
    case STRINGTV:
        print_string( output, pointer );
        break;
    case SYMBOLTV:
        print_string_contents( output, pointer );
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
                  L"Error: Unrecognised tag value %d (%c%c%c%c)\n",
                  cell.tag.value, cell.tag.bytes[0], cell.tag.bytes[1],
                  cell.tag.bytes[2], cell.tag.bytes[3] );
        break;
    }
}
