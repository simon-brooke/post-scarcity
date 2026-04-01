/**
 *  ops/string_ops.h
 *
 *  Operations on a Lisp string frame.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdint.h>

/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "debug.h"
#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/tags.h"

#include "ops/truth.h"

#include "payloads/exception.h"


/**
 * Return a hash value for this string like thing.
 *
 * What's important here is that two strings with the same characters in the
 * same order should have the same hash value, even if one was created using
 * `"foobar"` and the other by `(append "foo" "bar")`. I *think* this function
 * has that property. I doubt that it's the most efficient hash function to
 * have that property.
 *
 * returns 0 for things which are not string like.
 */
uint32_t calculate_hash( wint_t c, struct pso_pointer ptr ) {
    struct pso2 *cell = pointer_to_object( ptr );
    uint32_t result = 0;

    switch ( get_tag_value( ptr ) ) {
        case KEYTV:
        case STRINGTV:
        case SYMBOLTV:
            if ( nilp( cell->payload.string.cdr ) ) {
                result = ( uint32_t ) c;
            } else {
                result =
                    ( ( uint32_t ) c *
                      cell->payload.string.hash ) & 0xffffffff;
            }
            break;
    }

    return result;
}

 /**
 * Construct a string from this character (which later will be UTF) and
 * this tail. A string is implemented as a flat list of cells each of which
 * has one character and a pointer to the next; in the last cell the
 * pointer to next is nil.
 *
 * NOTE THAT: in 0.1.X, we may allocate symbols and keywords as arrays of 
 * wchar_t in larger pso classes, so this function may be only for strings
 * (and thus simpler).
 */
struct pso_pointer make_string_like_thing( wint_t c, struct pso_pointer tail,
                                           char *tag ) {
    struct pso_pointer pointer = nil;

    if ( check_type( tail, tag ) || check_tag( tail, NILTV ) ) {
        pointer = allocate( tag, CONS_SIZE_CLASS );
        struct pso2 *cell = pointer_to_object( pointer );

        cell->payload.string.character = c;
        cell->payload.string.cdr = tail;

        cell->payload.string.hash = calculate_hash( c, tail );
        debug_dump_object( pointer, DEBUG_ALLOC, 0 );
        debug_println( DEBUG_ALLOC );
    } else {
        // \todo should throw an exception!
        debug_printf( DEBUG_ALLOC, 0,
                      L"Warning: only %4.4s can be prepended to %4.4s\n",
                      tag, tag );
    }

    return pointer;
}

/**
 * Construct a string from the character `c` and this `tail`. A string is
 * implemented as a flat list of cells each of which has one character and a
 * pointer to the next; in the last cell the pointer to next is NIL.
 *
 * @param c the character to add (prepend);
 * @param tail the string which is being built.
 */
struct pso_pointer make_string( wint_t c, struct pso_pointer tail ) {
    return make_string_like_thing( c, tail, STRINGTAG );
}

/**
 * Construct a keyword from the character `c` and this `tail`. A string is
 * implemented as a flat list of cells each of which has one character and a
 * pointer to the next; in the last cell the pointer to next is NIL.
 *
 * @param c the character to add (prepend);
 * @param tail the keyword which is being built.
 */
struct pso_pointer make_keyword( wint_t c, struct pso_pointer tail ) {
    return make_string_like_thing( c, tail, KEYTAG );
}

/**
 * Construct a symbol from the character `c` and this `tail`. A string is
 * implemented as a flat list of cells each of which has one character and a
 * pointer to the next; in the last cell the pointer to next is NIL.
 *
 * @param c the character to add (prepend);
 * @param tail the symbol which is being built.
 */
struct pso_pointer make_symbol( wint_t c, struct pso_pointer tail ) {
    return make_string_like_thing( c, tail, STRINGTAG );
}


/**
 * Return a lisp string representation of this wide character string.
 */
struct pso_pointer c_string_to_lisp_string( wchar_t *string ) {
    struct pso_pointer result = nil;

    for ( int i = wcslen( string ) - 1; i >= 0; i-- ) {
        if ( iswprint( string[i] ) && string[i] != '"' ) {
            result = make_string( string[i], result );
        }
    }

    return result;
}

/**
 * Return a lisp keyword representation of this wide character string. In
 * keywords, I am accepting only lower case characters and numbers.
 */
struct pso_pointer c_string_to_lisp_keyword( wchar_t *symbol ) {
    struct pso_pointer result = nil;

    for ( int i = wcslen( symbol ) - 1; i >= 0; i-- ) {
        wchar_t c = towlower( symbol[i] );

        if ( iswalnum( c ) || c == L'-' ) {
            result = make_keyword( c, result );
        }
    }

    return result;
}
