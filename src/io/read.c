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

#include "memory/consspaceobject.h"
#include "debug.h"
#include "memory/dump.h"
#include "memory/hashmap.h"
#include "arith/integer.h"
#include "ops/intern.h"
#include "io/io.h"
#include "ops/lispops.h"
#include "arith/peano.h"
#include "io/print.h"
#include "arith/ratio.h"
#include "io/read.h"
#include "arith/real.h"
#include "memory/vectorspace.h"

/*
 * for the time being things which may be read are: 
 * * strings
 * * numbers - either integer, ratio or real
 * * lists 
 * * maps
 * * keywords
 * * atoms
 */

struct cons_pointer read_number( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 URL_FILE * input, wint_t initial,
                                 bool seen_period );
struct cons_pointer read_list( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               struct cons_pointer env,
                               URL_FILE * input, wint_t initial );
struct cons_pointer read_map( struct stack_frame *frame,
                              struct cons_pointer frame_pointer,
                              struct cons_pointer env,
                              URL_FILE * input, wint_t initial );
struct cons_pointer read_string( URL_FILE * input, wint_t initial );
struct cons_pointer read_symbol_or_key( URL_FILE * input, uint32_t tag,
                                        wint_t initial );

/**
 * quote reader macro in C (!)
 */
struct cons_pointer c_quote( struct cons_pointer arg ) {
    return make_cons( c_string_to_lisp_symbol( L"quote" ),
                      make_cons( arg, NIL ) );
}

/**
 * Read a path macro from the stream. A path macro is expected to be
 * 1. optionally a leading character such as '/' or '$', followed by
 * 2. one or more keywords with leading colons (':') but no intervening spaces; or
 * 3. one or more symbols separated by slashes; or
 * 4. keywords (with leading colons) interspersed with symbols (prefixed by slashes).
 */
struct cons_pointer read_path( URL_FILE * input, wint_t initial,
                               struct cons_pointer q ) {
    bool done = false;
    struct cons_pointer prefix = NIL;

    switch ( initial ) {
        case '/':
            prefix = c_string_to_lisp_symbol( L"oblist" );
            break;
        case '$':
        case L'§':
            prefix = c_string_to_lisp_symbol( L"session" );
            break;
    }

    while ( !done ) {
        wint_t c = url_fgetwc( input );
        if ( iswblank( c ) || iswcntrl( c ) ) {
            done = true;
        } else if ( url_feof( input ) ) {
            done = true;
        } else {
            switch ( c ) {
                case ':':
                    q = make_cons( read_symbol_or_key
                                   ( input, KEYTV, url_fgetwc( input ) ), q );
                    break;
                case '/':
                    q = make_cons( make_cons
                                   ( c_string_to_lisp_symbol( L"quote" ),
                                     make_cons( read_symbol_or_key
                                                ( input, SYMBOLTV,
                                                  url_fgetwc( input ) ),
                                                NIL ) ), q );
                    break;
                default:
                    if ( iswalpha( c ) ) {
                        q = make_cons( read_symbol_or_key
                                       ( input, SYMBOLTV, c ), q );
                    } else {
                        // TODO: it's really an error. Exception?
                        url_ungetwc( c, input );
                        done = true;
                    }
            }
        }
    }

    // right, we now have the path we want (reversed) in q.
    struct cons_pointer r = NIL;

    for ( struct cons_pointer p = q; !nilp( p ); p = c_cdr( p ) ) {
        r = make_cons( c_car( p ), r );
    }

    dec_ref( q );

    if ( !nilp( prefix ) ) {
        r = make_cons( prefix, r );
    }

    return make_cons( c_string_to_lisp_symbol( L"->" ), r );
}

/**
 * Read the next object on this input stream and return a cons_pointer to it,
 * treating this initial character as the first character of the object
 * representation.
 */
struct cons_pointer read_continuation( struct stack_frame *frame,
                                       struct cons_pointer frame_pointer,
                                       struct cons_pointer env,
                                       URL_FILE * input, wint_t initial ) {
    debug_print( L"entering read_continuation\n", DEBUG_IO );
    struct cons_pointer result = NIL;

    wint_t c;

    for ( c = initial;
          c == '\0' || iswblank( c ) || iswcntrl( c );
          c = url_fgetwc( input ) );

    if ( url_feof( input ) ) {
        result =
            throw_exception( c_string_to_lisp_string
                             ( L"End of file while reading" ), frame_pointer );
    } else {
        switch ( c ) {
            case ';':
                for ( c = url_fgetwc( input ); c != '\n';
                      c = url_fgetwc( input ) );
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
                             ( frame, frame_pointer, env, input,
                               url_fgetwc( input ) ) );
                break;
            case '(':
                result =
                    read_list( frame, frame_pointer, env, input,
                               url_fgetwc( input ) );
                break;
            case '{':
                result = read_map( frame, frame_pointer, env, input,
                                   url_fgetwc( input ) );
                break;
            case '"':
                result = read_string( input, url_fgetwc( input ) );
                break;
            case '-':{
                    wint_t next = url_fgetwc( input );
                    url_ungetwc( next, input );
                    if ( iswdigit( next ) ) {
                        result =
                            read_number( frame, frame_pointer, input, c,
                                         false );
                    } else {
                        result = read_symbol_or_key( input, SYMBOLTV, c );
                    }
                }
                break;
            case '.':
                {
                    wint_t next = url_fgetwc( input );
                    if ( iswdigit( next ) ) {
                        url_ungetwc( next, input );
                        result =
                            read_number( frame, frame_pointer, input, c,
                                         true );
                    } else if ( iswblank( next ) ) {
                        /* dotted pair. \todo this isn't right, we
                         * really need to backtrack up a level. */
                        result =
                            read_continuation( frame, frame_pointer, env,
                                               input, url_fgetwc( input ) );
                        debug_print
                            ( L"read_continuation: dotted pair; read cdr ",
                              DEBUG_IO );
                    } else {
                        read_symbol_or_key( input, SYMBOLTV, c );
                    }
                }
                break;
            case ':':
                result =
                    read_symbol_or_key( input, KEYTV, url_fgetwc( input ) );
                break;
            case '/':
                {
                    /* slash followed by whitespace is legit provided it's not
                     * preceded by anything - it's the division operator. Otherwise,
                     * it's terminal, probably part of a path, and needs pushed back.
                     */
                    wint_t cn = url_fgetwc( input );
                    if ( nilp( result )
                         && ( iswblank( cn ) || iswcntrl( cn ) ) ) {
                        url_ungetwc( cn, input );
                        result = make_symbol_or_key( c, NIL, SYMBOLTV );
                    } else {
                        url_ungetwc( cn, input );
                        result = read_path( input, c, NIL );
                    }
                }
                break;
            case '$':
            case L'§':
                result = read_path( input, c, NIL );
                break;
            default:
                if ( iswdigit( c ) ) {
                    result =
                        read_number( frame, frame_pointer, input, c, false );
                } else if ( iswprint( c ) ) {
                    result = read_symbol_or_key( input, SYMBOLTV, c );
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
 * \todo Need to do a lot of inc_ref and dec_ref, to make sure the
 * garbage is collected.
 */
struct cons_pointer read_number( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 URL_FILE * input,
                                 wint_t initial, bool seen_period ) {
    debug_print( L"entering read_number\n", DEBUG_IO );

    struct cons_pointer result = make_integer( 0, NIL );
    /* \todo we really need to be getting `base` from a privileged Lisp name -
     * and it should be the same privileged name we use when writing numbers */
    struct cons_pointer base = make_integer( 10, NIL );
    struct cons_pointer dividend = NIL;
    int places_of_decimals = 0;
    wint_t c;
    bool neg = initial == btowc( '-' );

    if ( neg ) {
        initial = url_fgetwc( input );
    }

    debug_printf( DEBUG_IO, L"read_number starting '%c' (%d)\n", initial,
                  initial );

    for ( c = initial; iswdigit( c )
          || c == L'.' || c == L'/' || c == L','; c = url_fgetwc( input ) ) {
        switch ( c ) {
            case L'.':
                if ( seen_period || !nilp( dividend ) ) {
                    return throw_exception( c_string_to_lisp_string
                                            ( L"Malformed number: too many periods" ),
                                            frame_pointer );
                } else {
                    debug_print( L"read_number: decimal point seen\n",
                                 DEBUG_IO );
                    seen_period = true;
                }
                break;
            case L'/':
                if ( seen_period || !nilp( dividend ) ) {
                    return throw_exception( c_string_to_lisp_string
                                            ( L"Malformed number: dividend of rational must be integer" ),
                                            frame_pointer );
                } else {
                    debug_print( L"read_number: ratio slash seen\n",
                                 DEBUG_IO );
                    dividend = result;

                    result = make_integer( 0, NIL );
                }
                break;
            case L',':
                // silently ignore it.
                break;
            default:
                result = add_integers( multiply_integers( result, base ),
                                       make_integer( ( int ) c - ( int ) '0',
                                                     NIL ) );

                debug_printf( DEBUG_IO,
                              L"read_number: added character %c, result now ",
                              c );
                debug_print_object( result, DEBUG_IO );
                debug_print( L"\n", DEBUG_IO );

                if ( seen_period ) {
                    places_of_decimals++;
                }
        }
    }

    /*
     * push back the character read which was not a digit
     */
    url_ungetwc( c, input );

    if ( seen_period ) {
        debug_print( L"read_number: converting result to real\n", DEBUG_IO );
        struct cons_pointer div = make_ratio( result,
                                              make_integer( powl
                                                            ( to_long_double
                                                              ( base ),
                                                              places_of_decimals ),
                                                            NIL ) );
        inc_ref( div );

        result = make_real( to_long_double( div ) );

        dec_ref( div );
    } else if ( integerp( dividend ) ) {
        debug_print( L"read_number: converting result to ratio\n", DEBUG_IO );
        result = make_ratio( dividend, result );
    }

    if ( neg ) {
        debug_print( L"read_number: converting result to negative\n",
                     DEBUG_IO );

        result = negative( result );
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
                               struct cons_pointer env,
                               URL_FILE * input, wint_t initial ) {
    struct cons_pointer result = NIL;
    wint_t c;

    if ( initial != ')' ) {
        debug_printf( DEBUG_IO,
                      L"read_list starting '%C' (%d)\n", initial, initial );
        struct cons_pointer car =
            read_continuation( frame, frame_pointer, env, input,
                               initial );

        /* skip whitespace */
        for ( c = url_fgetwc( input );
              iswblank( c ) || iswcntrl( c ); c = url_fgetwc( input ) );

        if ( c == L'.' ) {
            /* might be a dotted pair; indeed, if we rule out numbers with
             * initial periods, it must be a dotted pair. \todo Ought to check,
             * howerver, that there's only one form after the period. */
            result =
                make_cons( car,
                           c_car( read_list( frame,
                                             frame_pointer,
                                             env,
                                             input, url_fgetwc( input ) ) ) );
        } else {
            result =
                make_cons( car,
                           read_list( frame, frame_pointer, env, input, c ) );
        }
    } else {
        debug_print( L"End of list detected\n", DEBUG_IO );
    }

    return result;
}

struct cons_pointer read_map( struct stack_frame *frame,
                              struct cons_pointer frame_pointer,
                              struct cons_pointer env,
                              URL_FILE * input, wint_t initial ) {
    // set write ACL to true whilst creating to prevent GC churn
    struct cons_pointer result =
        make_hashmap( DFLT_HASHMAP_BUCKETS, NIL, TRUE );
    wint_t c = initial;

    while ( c != L'}' ) {
        struct cons_pointer key =
            read_continuation( frame, frame_pointer, env, input, c );

        /* skip whitespace */
        for ( c = url_fgetwc( input ); iswblank( c ) || iswcntrl( c );
              c = url_fgetwc( input ) );

        struct cons_pointer value =
            read_continuation( frame, frame_pointer, env, input, c );

        /* skip commaa and whitespace at this point. */
        for ( c = url_fgetwc( input );
              c == L',' || iswblank( c ) || iswcntrl( c );
              c = url_fgetwc( input ) );

        result =
            hashmap_put( result, key,
                         eval_form( frame, frame_pointer, value, env ) );
    }

    // default write ACL for maps should be NIL.
    pointer_to_vso( result )->payload.hashmap.write_acl = NIL;

    return result;
}

/**
 * Read a string. This means either a string delimited by double quotes
 * (is_quoted == true), in which case it may contain whitespace but may
 * not contain a double quote character (unless escaped), or one not
 * so delimited in which case it may not contain whitespace (unless escaped)
 * but may contain a double quote character (probably not a good idea!)
 */
struct cons_pointer read_string( URL_FILE * input, wint_t initial ) {
    struct cons_pointer cdr = NIL;
    struct cons_pointer result;
    switch ( initial ) {
        case '\0':
            result = NIL;
            break;
        case '"':
            /* making a string of the null character means we can have an empty
             * string. Just returning NIL here would make an empty string
             * impossible. */
            result = make_string( '\0', NIL );
            break;
        default:
            result =
                make_string( initial,
                             read_string( input, url_fgetwc( input ) ) );
            break;
    }

    return result;
}

struct cons_pointer read_symbol_or_key( URL_FILE * input, uint32_t tag,
                                        wint_t initial ) {
    struct cons_pointer cdr = NIL;
    struct cons_pointer result;
    switch ( initial ) {
        case '\0':
            result = make_symbol_or_key( initial, NIL, tag );
            break;
        case '"':
        case '\'':
            /* unwise to allow embedded quotation marks in symbols */
        case ')':
        case ':':
        case '/':
            /*
             * symbols and keywords may not include right-parenthesis,
             * slashes or colons.
             */
            result = NIL;
            /*
             * push back the character read
             */
            url_ungetwc( initial, input );
            break;
        default:
            if ( iswprint( initial )
                 && !iswblank( initial ) ) {
                result =
                    make_symbol_or_key( initial,
                                        read_symbol_or_key( input,
                                                            tag,
                                                            url_fgetwc
                                                            ( input ) ), tag );
            } else {
                result = NIL;
                /*
                 * push back the character read
                 */
                url_ungetwc( initial, input );
            }
            break;
    }

    debug_print( L"read_symbol_or_key returning\n", DEBUG_IO );
    debug_dump_object( result, DEBUG_IO );

    return result;
}

/**
 * Read the next object on this input stream and return a cons_pointer to it.
 */
struct cons_pointer read( struct
                          stack_frame
                          *frame, struct cons_pointer frame_pointer,
                          struct cons_pointer env, URL_FILE * input ) {
    return read_continuation( frame, frame_pointer, env, input,
                              url_fgetwc( input ) );
}
