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
/* safe_iop, as available in the Ubuntu repository, is this one:
 * https://code.google.com/archive/p/safe-iop/wikis/README.wiki
 * which is installed as `libsafe-iop-dev`. There is an alternate
 * implementation here: https://github.com/redpig/safe-iop/
 * which shares the same version number but is not compatible. */
#include <safe_iop.h>
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
const wchar_t hex_digits[16] = L"0123456789ABCDEF";

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
    debug_print( L"Entering make_integer\n", DEBUG_ARITH );

    if ( integerp( more ) || nilp( more ) ) {
        result = allocate_cell( INTEGERTAG );
        struct cons_space_object *cell = &pointer2cell( result );
        cell->payload.integer.value = value;
        cell->payload.integer.more = more;

    }

    debug_print( L"make_integer: returning\n", DEBUG_ARITH );
    debug_dump_object( result, DEBUG_ARITH );
    return result;
}

/**
 * Return the sum of the integers pointed to by `a` and `b`. If either isn't
 * an integer, will return nil.
 */
struct cons_pointer add_integers( struct cons_pointer a,
                                  struct cons_pointer b ) {
    debug_print( L"Entering add_integers\n", DEBUG_ARITH );

    struct cons_pointer result = NIL;
    int64_t carry = 0;

    if ( integerp( a ) && integerp( b ) ) {
        while ( !nilp( a ) || !nilp( b ) || carry != 0 ) {
            debug_print( L"add_integers: ", DEBUG_ARITH );
            debug_print_object( a, DEBUG_ARITH );
            debug_print( L" x ", DEBUG_ARITH );
            debug_print_object( b, DEBUG_ARITH );
            debug_printf( DEBUG_ARITH, L"; carry = %ld\n", carry );

            int64_t av =
                integerp( a ) ? pointer2cell( a ).payload.integer.value : 0;
            int64_t bv =
                integerp( b ) ? pointer2cell( b ).payload.integer.value : 0;

            int64_t rv = 0;

            if ( safe_add( &rv, av, bv ) ) {
                carry = 0;
            } else {
                // TODO: we're correctly detecting overflow, but not yet correctly
                // handling it.
                debug_printf( DEBUG_ARITH,
                              L"add_integers: 64 bit overflow; setting carry to %ld\n",
                              carry );
                carry = llabs( rv / LONG_MAX );
                rv = rv % LONG_MAX;
            }

            result = make_integer( rv, result );
            a = pointer2cell( a ).payload.integer.more;
            b = pointer2cell( b ).payload.integer.more;
        }
    }
    debug_print( L"add_integers returning: ", DEBUG_ARITH );
    debug_print_object( result, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

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
        debug_print( L"multiply_integers: ", DEBUG_ARITH );
        debug_print_object( a, DEBUG_ARITH );
        debug_print( L" x ", DEBUG_ARITH );
        debug_print_object( b, DEBUG_ARITH );
        debug_println( DEBUG_ARITH );

        while ( !nilp( a ) || !nilp( b ) || carry != 0 ) {
            int64_t av =
                integerp( a ) ? pointer2cell( a ).payload.integer.value : 1;
            int64_t bv =
                integerp( b ) ? pointer2cell( b ).payload.integer.value : 1;

            int64_t rv = 0;

            if ( safe_mul( &rv, av, bv ) ) {
                carry = 0;
            } else {
                // TODO: we're correctly detecting overflow, but not yet correctly
                // handling it.
                debug_printf( DEBUG_ARITH,
                              L"multiply_integers: 64 bit overflow; setting carry to %ld\n",
                              carry );
                carry = llabs( rv / LONG_MAX );
                rv = rv % LONG_MAX;
            }

            result = make_integer( rv, result );
            a = pointer2cell( a ).payload.integer.more;
            b = pointer2cell( b ).payload.integer.more;
        }
    }
    debug_print( L"multiply_integers returning: ", DEBUG_ARITH );
    debug_print_object( result, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

    return result;
}

/**
 * don't use; private to integer_to_string, and somewaht dodgy.
 */
struct cons_pointer integer_to_string_add_digit( int digit, int digits,
                                                 struct cons_pointer tail ) {
    digits++;
    wint_t character = ( wint_t ) hex_digits[digit];
    return ( digits % 3 == 0 ) ?
        make_string( L',', make_string( character,
                                        tail ) ) :
        make_string( character, tail );
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
    int digits = 0;

    if ( accumulator == 0 ) {
        result = c_string_to_lisp_string( L"0" );
    } else {
        while ( accumulator > 0 ) {
            debug_printf( DEBUG_IO,
                          L"integer_to_string: accumulator is %ld\n:",
                          accumulator );
            do {
                debug_printf( DEBUG_IO,
                              L"integer_to_string: digit is %ld, hexadecimal is %lc\n:",
                              accumulator % base,
                              hex_digits[accumulator % base] );

                result =
                    integer_to_string_add_digit( accumulator % base, digits++,
                                                 result );
                accumulator = accumulator / base;
            } while ( accumulator > base );

            if ( integerp( integer.payload.integer.more ) ) {
                integer = pointer2cell( integer.payload.integer.more );
                int64_t i = integer.payload.integer.value;

                /* TODO: I don't believe it's as simple as this! */
                accumulator += ( base * ( i % base ) );
                result =
                    integer_to_string_add_digit( accumulator % base, digits++,
                                                 result );
                accumulator += ( base * ( i / base ) );
            }
        }

        if ( is_negative ) {
            result = make_string( L'-', result );
        }
    }

    return result;
}
