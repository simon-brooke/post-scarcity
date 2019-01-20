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
const char *hex_digits = "0123456789ABCDEF";

/*
 * Doctrine from here on in is that ALL integers are bignums, it's just
 * that integers less than 65 bits are bignums of one cell only.
 */

/**
 * return the numeric value of the cell indicated by this `pointer`, as a C
 * primitive double, not as a cons_space_object. The indicated cell may in
 * principle be any kind of number; if it is not a number, will return `NAN`.
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
 * Allocate an integer cell representing this `value` and return a cons_pointer to it.
 * @param value an integer value;
 * @param more `NIL`, or a pointer to the more significant cell(s) of this number.
 * *NOTE* that if `more` is not `NIL`, `value` *must not* exceed `MAX_INTEGER`.
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
 * Internal to `operate_on_integers`, do not use.
 * @param c a pointer to a cell, assumed to be an integer cell;
 * @param op a character representing the operation: expectedto be either
 * '+' or '*'; behaviour with other values is undefined.
 * \see operate_on_integers
 */
__int128_t cell_value( struct cons_pointer c, char op, bool is_first_cell ) {
    long int val = nilp( c ) ? 0 : pointer2cell( c ).payload.integer.value;
    long int carry = is_first_cell ? 0 : ( MAX_INTEGER + 1 );

    __int128_t result = ( __int128_t ) integerp( c ) ?
        ( val == 0 ) ? carry : val : op == '*' ? 1 : 0;
    debug_printf( DEBUG_ARITH,
                  L"cell_value: raw value is %ld, op = '%c', is_first_cell = %s; returning ",
                  val, op, is_first_cell ? "true" : "false" );
    debug_print_128bit( result, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

    return result;
}

/**
 * internal workings of both `add_integers` and `multiply_integers` (and
 * possibly, later, other operations. Apply the operator `op` to the
 * integer arguments `a` and `b`, and return a pointer to the result. If
 * either `a` or `b` is not an integer, returns `NIL`.
 *
 * @param a a pointer to a cell, assumed to be an integer cell;
 * @param b a pointer to a cell, assumed to be an integer cell;
 * @param op a character representing the operation: expected to be either
 * '+' or '*'; behaviour with other values is undefined.
 * \see add_integers
 * \see multiply_integers
 */
/* \todo there is a significant bug here, which manifests in multiply but
 * may not manifest in add. The value in the least significant cell ends
 * up significantly WRONG, but the value in the more significant cell
 * ends up correct. */
struct cons_pointer operate_on_integers( struct cons_pointer a,
                                         struct cons_pointer b, char op ) {
    struct cons_pointer result = NIL;
    struct cons_pointer cursor = NIL;
    __int128_t carry = 0;
    bool is_first_cell = true;

    if ( integerp( a ) && integerp( b ) ) {
        debug_print( L"operate_on_integers: \n", DEBUG_ARITH );
        debug_dump_object( a, DEBUG_ARITH );
        debug_printf( DEBUG_ARITH, L" %c \n", op );
        debug_dump_object( b, DEBUG_ARITH );
        debug_println( DEBUG_ARITH );

        while ( !nilp( a ) || !nilp( b ) || carry != 0 ) {
            __int128_t av = cell_value( a, op, is_first_cell );
            __int128_t bv = cell_value( b, op, is_first_cell );

            /* slightly dodgy. `MAX_INTEGER` is substantially smaller than `LONG_MAX`, and
             * `LONG_MAX * LONG_MAX` =~ the maximum value for `__int128_t`. So if the carry
             * is very large (which I'm not certain whether it can be and am not
             * intellectually up to proving it this morning) adding the carry might
             * overflow `__int128_t`. Edge-case testing required.
             */
            __int128_t rv = NAN;

            switch ( op ) {
                case '*':
                    rv = av * ( bv + carry );
                    break;
                case '+':
                    rv = av + bv + carry;
                    break;
            }

            debug_printf( DEBUG_ARITH,
                          L"operate_on_integers: op = '%c'; av = ", op );
            debug_print_128bit( av, DEBUG_ARITH );
            debug_print( L"; bv = ", DEBUG_ARITH );
            debug_print_128bit( bv, DEBUG_ARITH );
            debug_print( L"; carry = ", DEBUG_ARITH );
            debug_print_128bit( carry, DEBUG_ARITH );
            debug_print( L"; rv = ", DEBUG_ARITH );
            debug_print_128bit( rv, DEBUG_ARITH );
            debug_print( L"\n", DEBUG_ARITH );


            if ( MAX_INTEGER >= rv ) {
                carry = 0;
            } else {
                // \todo we're correctly detecting overflow, but not yet correctly
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
            is_first_cell = false;
        }
    }

    debug_print( L"operate_on_integers returning:\n", DEBUG_ARITH );
    debug_dump_object( result, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

    return result;
}

/**
 * Return a pointer to an integer representing the sum of the integers
 * pointed to by `a` and `b`. If either isn't an integer, will return nil.
 */
struct cons_pointer add_integers( struct cons_pointer a,
                                  struct cons_pointer b ) {

    return operate_on_integers( a, b, '+' );
}

/**
 * Return a pointer to an integer representing the product of the integers
 * pointed to by `a` and `b`. If either isn't an integer, will return nil.
 */
struct cons_pointer multiply_integers( struct cons_pointer a,
                                       struct cons_pointer b ) {
    return operate_on_integers( a, b, '*' );
}

/**
 * don't use; private to integer_to_string, and somewaht dodgy.
 */
struct cons_pointer integer_to_string_add_digit( int digit, int digits,
                                                 struct cons_pointer tail ) {
    digits++;
    wint_t character = btowc( hex_digits[digit] );
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
/*
 * \todo this blows up when printing three-cell integers, but works fine
 * for two-cell. What's happening is that when we cross the barrier we
 * SHOULD print 2^120, but what we actually print is 2^117. H'mmm.
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
                accumulator += integer.payload.integer.value == 0 ?
                    MAX_INTEGER :
                    ( llabs( integer.payload.integer.value ) *
                      ( MAX_INTEGER + 1 ) );
                debug_print
                    ( L"integer_to_string: crossing cell boundary, accumulator is: ",
                      DEBUG_IO );
                debug_print_128bit( accumulator, DEBUG_IO );
                debug_println( DEBUG_IO );
            }

            do {
                int offset = ( int ) ( accumulator % base );
                debug_printf( DEBUG_IO,
                              L"integer_to_string: digit is %ld, hexadecimal is %c, accumulator is: ",
                              offset, hex_digits[offset] );
                debug_print_128bit( accumulator, DEBUG_IO );
                debug_println( DEBUG_IO );

                result =
                    integer_to_string_add_digit( offset, digits++, result );
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
