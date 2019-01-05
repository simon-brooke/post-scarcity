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

/*
 * The maximum value we will allow in an integer cell.
 */
#define MAX_INTEGER ((__int128_t)0x0fffffffffffffffL)

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
    debug_print( L"Entering make_integer\n", DEBUG_ALLOC );

    if ( integerp( more ) || nilp( more ) ) {
        result = allocate_cell( INTEGERTAG );
        struct cons_space_object *cell = &pointer2cell( result );
        cell->payload.integer.value = value;
        cell->payload.integer.more = more;

    }

    debug_print( L"make_integer: returning\n", DEBUG_ALLOC );
    debug_dump_object( result, DEBUG_ALLOC );
    return result;
}


/**
 * internal workings of both `add_integers` and `multiply_integers` (and
 * possibly, later, other operations. Apply the operator `op` to the
 * integer arguments `a` and `b`, and return a pointer to the result. If
 * either `a` or `b` is not an integer, returns `NIL`.
 */
struct cons_pointer operate_on_integers( struct cons_pointer a,
                                       struct cons_pointer b,
                                        char op) {
    struct cons_pointer result = NIL;
    struct cons_pointer cursor = NIL;
    __int128_t carry = 0;

    if ( integerp( a ) && integerp( b ) ) {
        debug_print( L"operate_on_integers: \n", DEBUG_ARITH );
        debug_dump_object( a, DEBUG_ARITH );
        debug_printf( DEBUG_ARITH, L" %c \n", op);
        debug_dump_object( b, DEBUG_ARITH );
        debug_println( DEBUG_ARITH );

        while ( !nilp( a ) || !nilp( b ) || carry != 0 ) {
            __int128_t av =
                ( __int128_t ) integerp( a ) ? pointer2cell( a ).
                payload.integer.value : op == '*' ? 1 : 0;
            __int128_t bv =
                ( __int128_t ) integerp( b ) ? pointer2cell( b ).
                payload.integer.value : op == '*' ? 1 : 0;

            /* slightly dodgy. `MAX_INTEGER` is substantially smaller than `LONG_MAX`, and
             * `LONG_MAX * LONG_MAX` =~ the maximum value for `__int128_t`. So if the carry
             * is very large (which I'm not certain whether it can be and am not
             * intellectually up to proving it this morning) adding the carry might
             * overflow `__int128_t`. Edge-case testing required.
             */
            __int128_t rv = NAN;

          switch (op) {
            case '*':
          rv = ( av * bv ) + carry;
            break;
            case '+':
            rv = av + bv + carry;
            break;
          }

            if ( MAX_INTEGER >= rv ) {
              carry = 0;
            } else {
              // TODO: we're correctly detecting overflow, but not yet correctly
              // handling it.
              carry = rv >> 60;
              debug_printf( DEBUG_ARITH,
                           L"operate_on_integers: 64 bit overflow; setting carry to %ld\n",
                           ( int64_t ) carry );
              rv &= MAX_INTEGER;
            }

            struct cons_pointer tail = make_integer( ( int64_t ) rv, NIL );

            if ( nilp( cursor ) ) {
              cursor = tail;
            } else {
              inc_ref( tail );
              /* yes, this is a destructive change - but the integer has not yet been released
                       * into the wild */
              struct cons_space_object *c = &pointer2cell( cursor );
              c->payload.integer.more = tail;
              cursor = tail;
            }

            if ( nilp( result ) ) {
              result = cursor;
            }

            a = pointer2cell( a ).payload.integer.more;
            b = pointer2cell( b ).payload.integer.more;
        }
    }

    debug_print( L"operate_on_integers returning:\n", DEBUG_ARITH );
    debug_dump_object( result, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

    return result;
}

/**
 * Return the sum of the integers pointed to by `a` and `b`. If either isn't
 * an integer, will return nil.
 */
struct cons_pointer add_integers( struct cons_pointer a,
                                  struct cons_pointer b ) {

  return operate_on_integers(a, b, '+');
}

/**
 * Return the product of the integers pointed to by `a` and `b`. If either isn't
 * an integer, will return nil.
 */
struct cons_pointer multiply_integers( struct cons_pointer a,
                                       struct cons_pointer b ) {
    return operate_on_integers( a, b, '*');
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
    __int128_t accumulator = llabs( integer.payload.integer.value );
    bool is_negative = integer.payload.integer.value < 0;
    int digits = 0;

    if ( accumulator == 0 && nilp( integer.payload.integer.more ) ) {
        result = c_string_to_lisp_string( L"0" );
    } else {
        while ( accumulator > 0 || !nilp( integer.payload.integer.more ) ) {
            if ( !nilp( integer.payload.integer.more ) ) {
                integer = pointer2cell( integer.payload.integer.more );
                accumulator +=
                    ( llabs( integer.payload.integer.value ) *
                      ( MAX_INTEGER + 1 ) );
            }

            debug_printf( DEBUG_IO,
                          L"integer_to_string: accumulator is %ld\n:",
                          accumulator );
            do {
                debug_printf( DEBUG_IO,
                              L"integer_to_string: digit is %ld, hexadecimal is %C\n:",
                              accumulator % base,
                              btowc(hex_digits[accumulator % base] ));

                result =
                    integer_to_string_add_digit( accumulator % base, digits++,
                                                 result );
                accumulator = accumulator / base;
            } while ( accumulator > base );
        }

        if ( stringp( result )
             && pointer2cell( result ).payload.string.character == L',' ) {
            /* if the number of digits in the string is divisible by 3, there will be
             * an unwanted comma on the front. */
            struct cons_pointer tmp = result;
            result = pointer2cell( result ).payload.string.cdr;
            dec_ref( tmp );
        }

        if ( is_negative ) {
            result = make_string( L'-', result );
        }
    }

    return result;
}
