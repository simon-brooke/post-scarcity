/*
 * integer.c
 *
 * functions for integer cells.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#define _GNU_SOURCE
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "conspage.h"
#include "consspaceobject.h"
#include "debug.h"

/**
 * hexadecimal digits for printing numbers.
 */
const wchar_t *hex_digits = L"0123456789ABCDEF";

/*
 * Doctrine from here on in is that ALL integers are bignums, it's just
 * that integers less than 65 bits are bignums of one cell only.
 *
 * TODO: I have no idea at all how I'm going to print bignums!
 */

/**
 * return the numeric value of this cell, as a C primitive double, not
 * as a cons-space object. Cell may in principle be any kind of number.
 */
long double numeric_value( struct cons_pointer pointer ) {
    long double result = NAN;
    struct cons_space_object *cell = &pointer2cell( pointer );

    switch ( cell->tag.value ) {
        case INTEGERTV:
            result = 1.0;
            while ( cell->tag.value == INTEGERTV ) {
                result = ( result * LONG_MAX * cell->payload.integer.value );
                cell = &pointer2cell( cell->payload.integer.more );
            }
            break;
        case RATIOTV:
            result = numeric_value( cell->payload.ratio.dividend ) /
                numeric_value( cell->payload.ratio.divisor );
            break;
        case REALTV:
            result = cell->payload.real.value;
            break;
            // default is NAN
    }

    return result;
}

/**
 * Allocate an integer cell representing this value and return a cons pointer to it.
 */
struct cons_pointer make_integer( int64_t value, struct cons_pointer more ) {
    struct cons_pointer result = NIL;

    if ( integerp( more ) || nilp( more ) ) {
        result = allocate_cell( INTEGERTAG );
        struct cons_space_object *cell = &pointer2cell( result );
        cell->payload.integer.value = value;
        cell->payload.integer.more = more;

        debug_dump_object( result, DEBUG_ARITH );
    }

    return result;
}

/**
 * Return the sum of the integers pointed to by `a` and `b`. If either isn't
 * an integer, will return nil.
 */
struct cons_pointer add_integers( struct cons_pointer a,
                                  struct cons_pointer b ) {
    struct cons_pointer result = NIL;
    int64_t carry = 0;

    if ( integerp( a ) && integerp( b ) ) {
        while ( !nilp( a ) || !nilp( b ) || carry != 0 ) {
            int64_t av =
                integerp( a ) ? pointer2cell( a ).payload.integer.value : 0;
            int64_t bv =
                integerp( b ) ? pointer2cell( b ).payload.integer.value : 0;

            __int128_t rv = av + bv + carry;

            if ( rv > LONG_MAX || rv < LONG_MIN ) {
                carry = llabs( rv / LONG_MAX );
                rv = rv % LONG_MAX;
            } else {
                carry = 0;
            }

            result = make_integer( rv, result );
            a = pointer2cell( a ).payload.integer.more;
            b = pointer2cell( b ).payload.integer.more;
        }
    }

    return result;
}

/**
 * Return the product of the integers pointed to by `a` and `b`. If either isn't
 * an integer, will return nil.
 */
struct cons_pointer multiply_integers( struct cons_pointer a,
                                       struct cons_pointer b ) {
    struct cons_pointer result = NIL;
    int64_t carry = 0;

    if ( integerp( a ) && integerp( b ) ) {
        while ( !nilp( a ) || !nilp( b ) || carry != 0 ) {
            int64_t av =
                integerp( a ) ? pointer2cell( a ).payload.integer.value : 1;
            int64_t bv =
                integerp( b ) ? pointer2cell( b ).payload.integer.value : 1;

            __int128_t rv = ( av * bv ) + carry;

            if ( rv > LONG_MAX || rv < LONG_MIN ) {
                carry = llabs( rv / LONG_MAX );
                rv = rv % LONG_MAX;
            } else {
                carry = 0;
            }

            result = make_integer( rv, result );
            a = pointer2cell( a ).payload.integer.more;
            b = pointer2cell( b ).payload.integer.more;
        }
    }

    return result;
}

/**
 * The general principle of printing a bignum is that you print the least
 * significant digit in whatever base you're dealing with, divide through
 * by the base, print the next, and carry on until you've none left.
 * Obviously, that means you print from right to left. Given that we build
 * strings from right to left, 'printing' an integer to a lisp string
 * would seem reasonably easy. The problem is when you jump from one integer
 * object to the next. 64 bit integers don't align with decimal numbers, so
 * when we get to the last digit from one integer cell, we have potentially
 * to be looking to the next. H'mmmm.
 */
struct cons_pointer integer_to_string( struct cons_pointer int_pointer,
                                       int base ) {
    struct cons_pointer result = NIL;
    struct cons_space_object integer = pointer2cell( int_pointer );
    int64_t accumulator = integer.payload.integer.value;
    bool is_negative = accumulator < 0;
    accumulator = llabs( accumulator );

    while ( accumulator > 0 ) {
        while ( accumulator > base ) {
            result = make_string( hex_digits[accumulator % base], result );
            accumulator = accumulator / base;
        }

        if ( integerp( integer.payload.integer.more ) ) {
            integer = pointer2cell( integer.payload.integer.more );
            int64_t i = integer.payload.integer.value;

            /* TODO: I don't believe it's as simple as this! */
            accumulator += ( base * ( i % base ) );
            result = make_string( hex_digits[accumulator % base], result );
            accumulator += ( base * ( i / base ) );
        }
    }

    if ( is_negative ) {
        result = make_string( L'-', result );
    }

    return result;
}
