/*
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
#include "debug.h"
#include "dump.h"
#include "integer.h"
#include "intern.h"
#include "lispops.h"
#include "print.h"
#include "ratio.h"
#include "read.h"
#include "real.h"
#include "vectorspace.h"

/*
 * for the time being things which may be read are: strings numbers - either
 * integer or real, but not yet including ratios or bignums lists Can't read
 * atoms because I don't yet know what an atom is or how it's stored.
 */

struct cons_pointer read_number( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 FILE * input, wint_t initial,
                                 bool seen_period );
struct cons_pointer read_list( struct stack_frame *frame,
                               struct cons_pointer frame_pointer, FILE * input,
                               wint_t initial );
struct cons_pointer read_string( FILE * input, wint_t initial );
struct cons_pointer read_symbol( FILE * input, wint_t initial );

/**
 * quote reader macro in C (!)
 */
struct cons_pointer c_quote( struct cons_pointer arg ) {
    return make_cons( c_string_to_lisp_symbol( L"quote" ),
                      make_cons( arg, NIL ) );
}

/**
 * Read the next object on this input stream and return a cons_pointer to it,
 * treating this initial character as the first character of the object
 * representation.
 */
struct cons_pointer read_continuation( struct stack_frame *frame,
                                       struct cons_pointer frame_pointer,
                                       FILE * input, wint_t initial ) {
    debug_print( L"entering read_continuation\n", DEBUG_IO );
    struct cons_pointer result = NIL;

    wint_t c;

    for ( c = initial;
          c == '\0' || iswblank( c ) || iswcntrl( c ); c = fgetwc( input ) );

    if ( feof( input ) ) {
        result =
            throw_exception( c_string_to_lisp_string
                             ( L"End of file while reading" ), frame_pointer );
    } else {
        switch ( c ) {
            case ';':
                for ( c = fgetwc( input ); c != '\n'; c = fgetwc( input ) );
                /* skip all characters from semi-colon to the end of the line */
                break;
            case EOF:
                result = throw_exception( c_string_to_lisp_string
                                          ( L"End of input while reading" ),
                                          frame_pointer );
                break;
            case '\'':
                result =
                    c_quote( read_continuation
                             ( frame, frame_pointer, input,
                               fgetwc( input ) ) );
                break;
            case '(':
                result =
                    read_list( frame, frame_pointer, input, fgetwc( input ) );
                break;
            case '"':
                result = read_string( input, fgetwc( input ) );
                break;
            case '-':{
                    wint_t next = fgetwc( input );
                    ungetwc( next, input );
                    if ( iswdigit( next ) ) {
                        result =
                            read_number( frame, frame_pointer, input, c,
                                         false );
                    } else {
                        result = read_symbol( input, c );
                    }
                }
                break;
            case '.':
                {
                    wint_t next = fgetwc( input );
                    if ( iswdigit( next ) ) {
                        ungetwc( next, input );
                        result =
                            read_number( frame, frame_pointer, input, c,
                                         true );
                    } else if ( iswblank( next ) ) {
                        /* dotted pair. TODO: this isn't right, we
                         * really need to backtrack up a level. */
                        result =
                            read_continuation( frame, frame_pointer, input,
                                               fgetwc( input ) );
                    } else {
                        read_symbol( input, c );
                    }
                }
                break;
            default:
                if ( iswdigit( c ) ) {
                    result =
                        read_number( frame, frame_pointer, input, c, false );
                } else if ( iswprint( c ) ) {
                    result = read_symbol( input, c );
                } else {
                    result =
                        throw_exception( make_cons( c_string_to_lisp_string
                                                    ( L"Unrecognised start of input character" ),
                                                    make_string( c, NIL ) ),
                                         frame_pointer );
                }
                break;
        }
    }
    debug_print( L"read_continuation returning\n", DEBUG_IO );
    debug_dump_object( result, DEBUG_IO );

    return result;
}

/**
 * read a number from this input stream, given this initial character.
 * TODO: to be able to read bignums, we need to read the number from the
 * input stream into a Lisp string, and then convert it to a number.
 */
struct cons_pointer read_number( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 FILE * input,
                                 wint_t initial, bool seen_period ) {
    debug_print( L"entering read_number\n", DEBUG_IO );
    struct cons_pointer result = NIL;
    int64_t accumulator = 0;
    int64_t dividend = 0;
    int places_of_decimals = 0;
    wint_t c;
    bool negative = initial == btowc( '-' );

    if ( negative ) {
        initial = fgetwc( input );
    }

    debug_printf( DEBUG_IO, L"read_number starting '%c' (%d)\n", initial,
                  initial );

    for ( c = initial; iswdigit( c )
          || c == L'.' || c == L'/' || c == L','; c = fgetwc( input ) ) {
        if ( c == btowc( '.' ) ) {
            if ( seen_period || dividend != 0 ) {
                return throw_exception( c_string_to_lisp_string
                                        ( L"Malformed number: too many periods" ),
                                        frame_pointer );
            } else {
                seen_period = true;
            }
        } else if ( c == btowc( '/' ) ) {
            if ( seen_period || dividend > 0 ) {
                return throw_exception( c_string_to_lisp_string
                                        ( L"Malformed number: dividend of rational must be integer" ),
                                        frame_pointer );
            } else {
                dividend = negative ? 0 - accumulator : accumulator;

                accumulator = 0;
            }
        } else if ( c == L',' ) {
            // silently ignore it.
        } else {
            accumulator = accumulator * 10 + ( ( int ) c - ( int ) '0' );

            debug_printf( DEBUG_IO,
                          L"Added character %c, accumulator now %ld\n",
                          c, accumulator );

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
        long double rv = ( long double )
            ( accumulator / pow( 10, places_of_decimals ) );
        if ( negative ) {
            rv = 0 - rv;
        }
        result = make_real( rv );
    } else if ( dividend != 0 ) {
        result =
            make_ratio( frame_pointer, make_integer( dividend, NIL ),
                        make_integer( accumulator, NIL ) );
    } else {
        if ( negative ) {
            accumulator = 0 - accumulator;
        }
        result = make_integer( accumulator, NIL );
    }

    debug_print( L"read_number returning\n", DEBUG_IO );
    debug_dump_object( result, DEBUG_IO );

    return result;
}

/**
 * Read a list from this input stream, which no longer contains the opening
 * left parenthesis.
 */
struct cons_pointer read_list( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               FILE * input, wint_t initial ) {
    struct cons_pointer result = NIL;
    if ( initial != ')' ) {
        debug_printf( DEBUG_IO,
                      L"read_list starting '%C' (%d)\n", initial, initial );
        struct cons_pointer car =
            read_continuation( frame, frame_pointer, input,
                               initial );
        result =
            make_cons( car,
                       read_list( frame, frame_pointer, input,
                                  fgetwc( input ) ) );
    } else {
        debug_print( L"End of list detected\n", DEBUG_IO );
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
            /* making a string of the null character means we can have an empty
             * string. Just returning NIL here would make an empty string
             * impossible. */
            result = make_string( '\0', NIL );
            break;
        default:
            result =
                make_string( initial, read_string( input, fgetwc( input ) ) );
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
            result =
                make_symbol( initial, read_symbol( input, fgetwc( input ) ) );
            break;
        case ')':
            /*
             * symbols may not include right-parenthesis
             */
            result = NIL;
            /*
             * push back the character read
             */
            ungetwc( initial, input );
            break;
        default:
            if ( iswprint( initial )
                 && !iswblank( initial ) ) {
                result =
                    make_symbol( initial,
                                 read_symbol( input, fgetwc( input ) ) );
            } else {
                result = NIL;
                /*
                 * push back the character read
                 */
                ungetwc( initial, input );
            }
            break;
    }

    debug_print( L"read_symbol returning\n", DEBUG_IO );
    debug_dump_object( result, DEBUG_IO );

    return result;
}

/**
 * Read the next object on this input stream and return a cons_pointer to it.
 */
struct cons_pointer read( struct
                          stack_frame
                          *frame, struct cons_pointer frame_pointer,
                          FILE * input ) {
    return read_continuation( frame, frame_pointer, input, fgetwc( input ) );
}
