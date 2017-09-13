/**
 * read.c
 *
 * First pass at a reader, for bootstrapping.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "consspaceobject.h"
#include "integer.h"
#include "intern.h"
#include "read.h"
#include "real.h"

/*
 * for the time being things which may be read are: strings numbers - either
 * integer or real, but not yet including ratios or bignums lists Can't read
 * atoms because I don't yet know what an atom is or how it's stored.
 */

struct cons_pointer read_number( FILE * input, wint_t initial );
struct cons_pointer read_list( FILE * input, wint_t initial );
struct cons_pointer read_string( FILE * input, wint_t initial );
struct cons_pointer read_symbol( FILE * input, wint_t initial );

/**
 * quote reader macro in C (!)
 */
struct cons_pointer c_quote( struct cons_pointer arg ) {
    return make_cons( c_string_to_lisp_symbol( "quote" ),
                      make_cons( arg, NIL ) );
}

/**
 * Read the next object on this input stream and return a cons_pointer to it,
 * treating this initial character as the first character of the object
 * representation.
 */
struct cons_pointer read_continuation( FILE * input, wint_t initial ) {
    struct cons_pointer result = NIL;

    wint_t c;

    for ( c = initial;
          c == '\0' || iswblank( c ) || iswcntrl( c ); c = fgetwc( input ) );

    switch ( c ) {
    case '\'':
        result = c_quote( read_continuation( input, fgetwc( input ) ) );
        break;
    case '(':
        result = read_list( input, fgetwc( input ) );
        break;
    case '"':
        result = read_string( input, fgetwc( input ) );
        break;
    default:
        if ( iswdigit( c ) ) {
            result = read_number( input, c );
        } else if ( iswprint( c ) ) {
            result = read_symbol( input, c );
        } else {
            fprintf( stderr, "Unrecognised start of input character %c\n", c );
        }
    }

    return result;
}

/**
 * read a number from this input stream, given this initial character.
 */
struct cons_pointer read_number( FILE * input, wint_t initial ) {
  struct cons_pointer result = NIL;
    long int accumulator = 0;
    int places_of_decimals = 0;
    bool seen_period = false;
    wint_t c;

    fprintf( stderr, "read_number starting '%c' (%d)\n", initial, initial );

    for ( c = initial; iswdigit( c ) || c == btowc( '.' );
          c = fgetwc( input ) ) {
        if ( c == btowc( '.' ) ) {
            seen_period = true;
        } else {
            accumulator = accumulator * 10 + ( ( int ) c - ( int ) '0' );

            fwprintf( stderr, L"Added character %c, accumulator now %ld\n", c,
                     accumulator );

            if ( seen_period ) {
                places_of_decimals++;
            }
        }
    }

    /*
     * push back the character read which was not a digit
     */
    ungetwc( c, input );

    if ( seen_period ) {
      long double rv =  (long double)
	( accumulator / pow(10, places_of_decimals) );

      fwprintf( stderr, L"read_numer returning %Lf\n", rv);
        result = make_real( rv);
    } else {
        result = make_integer( accumulator );
    }

    return result;
}

/**
 * Read a list from this input stream, which no longer contains the opening
 * left parenthesis.
 */
struct cons_pointer read_list( FILE * input, wint_t initial ) {
    struct cons_pointer result = NIL;

    if ( initial != ')' ) {
        fwprintf( stderr, L"read_list starting '%C' (%d)\n", initial,
                  initial );
        struct cons_pointer car = read_continuation( input, initial );
        result = make_cons( car, read_list( input, fgetwc( input ) ) );
    } else {
        fprintf( stderr, "End of list detected\n" );
    }

    return result;
}

/**
 * Read a string. This means either a string delimited by double quotes
 * (is_quoted == true), in which case it may contain whitespace but may
 * not contain a double quote character (unless escaped), or one not
 * so delimited in which case it may not contain whitespace (unless escaped)
 * but may contain a double quote character (probably not a good idea!)
 */
struct cons_pointer read_string( FILE * input, wint_t initial ) {
    struct cons_pointer cdr = NIL;
    struct cons_pointer result;

    switch ( initial ) {
    case '\0':
        result = make_string( initial, NIL );
        break;
    case '"':
        result = make_string( '\0', NIL );
        break;
    default:
        result = make_string( initial, read_string( input, fgetwc( input ) ) );
        break;
    }

    return result;
}

struct cons_pointer read_symbol( FILE * input, wint_t initial ) {
    struct cons_pointer cdr = NIL;
    struct cons_pointer result;

    switch ( initial ) {
    case '\0':
        result = make_symbol( initial, NIL );
        break;
    case '"':
        /*
         * THIS IS NOT A GOOD IDEA, but is legal
         */
        result = make_symbol( initial, read_symbol( input, fgetwc( input ) ) );
        break;
    case ')':
        /*
         * unquoted strings may not include right-parenthesis
         */
        result = make_symbol( '\0', NIL );
        /*
         * push back the character read
         */
        ungetwc( initial, input );
        break;
    default:
        if ( iswalnum( initial ) ) {
            result =
                make_symbol( initial, read_symbol( input, fgetwc( input ) ) );
        } else {
            result = NIL;
            /*
             * push back the character read
             */
            ungetwc( initial, input );
        } 
        break;
    }

    return result;
}

/**
 * Read the next object on this input stream and return a cons_pointer to it.
 */
struct cons_pointer read( FILE * input ) {
    return read_continuation( input, fgetwc( input ) );
}
