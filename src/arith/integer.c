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

#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "debug.h"
#include "ops/equal.h"
#include "ops/lispops.h"
#include "arith/peano.h"

/**
 * hexadecimal digits for printing numbers.
 */
const char *hex_digits = "0123456789ABCDEF";

/*
 * Doctrine from here on in is that ALL integers are bignums, it's just
 * that integers less than 65 bits are bignums of one cell only.
 */

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
        result = allocate_cell( INTEGERTV );
        struct cons_space_object *cell = &pointer2cell( result );
        cell->payload.integer.value = value;
        cell->payload.integer.more = more;
    }

    debug_print( L"make_integer: returning\n", DEBUG_ALLOC );
    debug_dump_object( result, DEBUG_ALLOC );
    return result;
}

/**
 * Low level integer arithmetic, do not use elsewhere.
 *
 * @param c a pointer to a cell, assumed to be an integer cell;
 * @param op a character representing the operation: expectedto be either
 * '+' or '*'; behaviour with other values is undefined.
 * @param is_first_cell true if this is the first cell in a bignum
 * chain, else false.
 * \see multiply_integers
 * \see add_integers
 */
__int128_t cell_value( struct cons_pointer c, char op, bool is_first_cell ) {
    long int val = nilp( c ) ? 0 : pointer2cell( c ).payload.integer.value;

    long int carry = is_first_cell ? 0 : ( MAX_INTEGER + 1 );

    __int128_t result = ( __int128_t ) integerp( c ) ?
        ( val == 0 ) ? carry : val : op == '*' ? 1 : 0;
    debug_printf( DEBUG_ARITH,
                  L"cell_value: raw value is %ld, is_first_cell = %s; '%4.4s'; returning ",
                  val, is_first_cell ? "true" : "false",
                  pointer2cell( c ).tag.bytes );
    debug_print_128bit( result, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

    return result;
}

/**
 * Overwrite the value field of the integer indicated by `new` with
 * the least significant 60 bits of `val`, and return the more significant
 * bits (if any) right-shifted by 60 places. Destructive, primitive, do not
 * use in any context except primitive operations on integers.
 *
 * @param val the value to represent;
 * @param less_significant the less significant words of this bignum, if any,
 * else NIL;
 * @param new a newly created integer, which will be destructively changed.
 * @return carry, if any, else 0.
 */
__int128_t int128_to_integer( __int128_t val,
                              struct cons_pointer less_significant,
                              struct cons_pointer new ) {
    __int128_t carry = 0;

    if ( MAX_INTEGER >= val ) {
        carry = 0;
    } else {
        carry = val >> INTEGER_BIT_SHIFT; 
        debug_printf( DEBUG_ARITH,
                      L"int128_to_integer: 64 bit overflow; setting carry to %ld\n",
                      ( int64_t ) carry );
        val &= MAX_INTEGER;
    }

    struct cons_space_object *newc = &pointer2cell( new );
    newc->payload.integer.value = val;

    if ( integerp( less_significant ) ) {
        struct cons_space_object *lsc = &pointer2cell( less_significant );
        inc_ref( new );
        lsc->payload.integer.more = new;
    }

    return carry;
}

struct cons_pointer make_integer_128( __int128_t val,
                                      struct cons_pointer less_significant ) {
    struct cons_pointer result = NIL;

    do {
        if ( MAX_INTEGER >= val ) {
            result = make_integer( ( long int ) val, less_significant );
        } else {
            less_significant =
                make_integer( ( long int ) val & MAX_INTEGER,
                              less_significant );
            val = val >> INTEGER_BIT_SHIFT;
        }

    } while ( nilp( result ) );

    return result;
}

/**
 * Return a pointer to an integer representing the sum of the integers
 * pointed to by `a` and `b`. If either isn't an integer, will return nil.
 */
struct cons_pointer add_integers( struct cons_pointer a,
                                  struct cons_pointer b ) {
    struct cons_pointer result = NIL;
    struct cons_pointer cursor = NIL;

    debug_print( L"add_integers: a = ", DEBUG_ARITH );
    debug_print_object( a, DEBUG_ARITH );
    debug_print( L"; b = ", DEBUG_ARITH );
    debug_print_object( b, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

    __int128_t carry = 0;
    bool is_first_cell = true;

    if ( integerp( a ) && integerp( b ) ) {
        debug_print( L"add_integers: \n", DEBUG_ARITH );
        debug_dump_object( a, DEBUG_ARITH );
        debug_print( L" plus \n", DEBUG_ARITH );
        debug_dump_object( b, DEBUG_ARITH );
        debug_println( DEBUG_ARITH );

        while ( !nilp( a ) || !nilp( b ) || carry != 0 ) {
            __int128_t av = cell_value( a, '+', is_first_cell );
            __int128_t bv = cell_value( b, '+', is_first_cell );
            __int128_t rv = av + bv + carry;

            debug_print( L"add_integers: av = ", DEBUG_ARITH );
            debug_print_128bit( av, DEBUG_ARITH );
            debug_print( L"; bv = ", DEBUG_ARITH );
            debug_print_128bit( bv, DEBUG_ARITH );
            debug_print( L"; carry = ", DEBUG_ARITH );
            debug_print_128bit( carry, DEBUG_ARITH );
            debug_print( L"; rv = ", DEBUG_ARITH );
            debug_print_128bit( rv, DEBUG_ARITH );
            debug_print( L"\n", DEBUG_ARITH );

            struct cons_pointer new = make_integer( 0, NIL );
            carry = int128_to_integer( rv, cursor, new );
            cursor = new;

            if ( nilp( result ) ) {
                result = cursor;
            }

            a = pointer2cell( a ).payload.integer.more;
            b = pointer2cell( b ).payload.integer.more;
            is_first_cell = false;
        }
    }

    debug_print( L"add_integers returning: ", DEBUG_ARITH );
    debug_print_object( result, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

    return result;
}

struct cons_pointer base_partial( int depth ) {
    struct cons_pointer result = NIL;

    for ( int i = 0; i < depth; i++ ) {
        result = make_integer( 0, result );
    }

    return result;
}

/**
 * destructively modify this `partial` by appending this `digit`.
 */
struct cons_pointer append_digit( struct cons_pointer partial,
                                  struct cons_pointer digit ) {
    struct cons_pointer c = partial;
    struct cons_pointer result = partial;

    if ( nilp( partial ) ) {
        result = digit;
    } else {
        while ( !nilp( pointer2cell( c ).payload.integer.more ) ) {
            c = pointer2cell( c ).payload.integer.more;
        }

        ( &pointer2cell( c ) )->payload.integer.more = digit;
    }
    return result;
}



/**
 * Return a pointer to an integer representing the product of the integers
 * pointed to by `a` and `b`. If either isn't an integer, will return nil.
 *
 * Yes, this is one of Muhammad ibn Musa al-Khwarizmi's original recipes, so
 * you'd think it would be easy; the reason that each step is documented is
 * because I did not find it so.
 *
 * @param a an integer;
 * @param b an integer.
 */
struct cons_pointer multiply_integers( struct cons_pointer a,
                                       struct cons_pointer b ) {
    struct cons_pointer result = make_integer( 0, NIL );
    bool neg = is_negative( a ) != is_negative( b );
    bool is_first_b = true;
    int i = 0;

    debug_print( L"multiply_integers: a = ", DEBUG_ARITH );
    debug_print_object( a, DEBUG_ARITH );
    debug_print( L"; b = ", DEBUG_ARITH );
    debug_print_object( b, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

    if ( integerp( a ) && integerp( b ) ) {
        /* for each digit in a, starting with the least significant (ai) */

        for ( struct cons_pointer ai = a; !nilp( ai );
              ai = pointer2cell( ai ).payload.integer.more ) {
            /* set carry to 0 */
            __int128_t carry = 0;

            /* set least significant digits for result ri for this iteration
             * to i zeros */
            struct cons_pointer ri = base_partial( i++ );

            /* for each digit in b, starting with the least significant (bj) */
            for ( struct cons_pointer bj = b; !nilp( bj );
                  bj = pointer2cell( bj ).payload.integer.more ) {

                debug_printf( DEBUG_ARITH,
                              L"multiply_integers: a[i] = %Ld, b[j] = %Ld, i = %d\n",
                              pointer2cell( ai ).payload.integer.value,
                              pointer2cell( bj ).payload.integer.value, i );

                /* multiply ai with bj and add the carry, resulting in a
                 * value xj which may exceed one digit */
                __int128_t xj = pointer2cell( ai ).payload.integer.value *
                    pointer2cell( bj ).payload.integer.value;
                xj += carry;

                /* if xj exceeds one digit, break it into the digit dj and
                 * the carry */
                carry = xj >> INTEGER_BIT_SHIFT;
                struct cons_pointer dj = make_integer( xj & MAX_INTEGER, NIL );

                /* destructively modify ri by appending dj */
                ri = append_digit( ri, dj );
            }                   /* end for bj */

            /* if carry is not equal to zero, append it as a final digit
             * to ri */
            if ( carry != 0 ) {
                ri = append_digit( ri, make_integer( carry, NIL ) );
            }

            /* add ri to result */
            result = add_integers( result, ri );

            debug_print( L"multiply_integers: result is ", DEBUG_ARITH );
            debug_print_object( result, DEBUG_ARITH );
            debug_println( DEBUG_ARITH );
        }                       /* end for ai */
    }

    debug_print( L"multiply_integers returning: ", DEBUG_ARITH );
    debug_print_object( result, DEBUG_ARITH );
    debug_println( DEBUG_ARITH );

    return result;
}

/**
 * don't use; private to integer_to_string, and somewhat dodgy.
 */
struct cons_pointer integer_to_string_add_digit( int digit, int digits,
                                                 struct cons_pointer tail ) {
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
struct cons_pointer integer_to_string( struct cons_pointer int_pointer,
                                       int base ) {
    struct cons_pointer result = NIL;

    if ( integerp( int_pointer ) ) {
        struct cons_pointer next =
            pointer2cell( int_pointer ).payload.integer.more;
        __int128_t accumulator =
            llabs( pointer2cell( int_pointer ).payload.integer.value );
        bool is_negative =
            pointer2cell( int_pointer ).payload.integer.value < 0;
        int digits = 0;

        if ( accumulator == 0 && nilp( next ) ) {
            result = c_string_to_lisp_string( L"0" );
        } else {
            while ( accumulator > 0 || !nilp( next ) ) {
                if ( accumulator < MAX_INTEGER && !nilp( next ) ) {
                    accumulator +=
                        ( pointer2cell( next ).payload.integer.value << INTEGER_BIT_SHIFT );
                    next = pointer2cell( next ).payload.integer.more;
                }
                int offset = ( int ) ( accumulator % base );
                debug_printf( DEBUG_IO,
                              L"integer_to_string: digit is %ld, hexadecimal is %c, accumulator is: ",
                              offset, hex_digits[offset] );
                debug_print_128bit( accumulator, DEBUG_IO );
                debug_print( L"; result is: ", DEBUG_IO );
                debug_print_object( result, DEBUG_IO );
                debug_println( DEBUG_IO );

                result =
                    integer_to_string_add_digit( offset, ++digits, result );
                accumulator = accumulator / base;
            }

            if ( stringp( result )
                 && pointer2cell( result ).payload.string.character == L',' ) {
                /* if the number of digits in the string is divisible by 3, there will be
                 * an unwanted comma on the front. */
                result = pointer2cell( result ).payload.string.cdr;
            }


            if ( is_negative ) {
                result = make_string( L'-', result );
            }
        }
    }

    return result;
}

/**
 * true if a and be are both integers whose value is the same value.
 */
bool equal_integer_integer( struct cons_pointer a, struct cons_pointer b ) {
    bool result = false;

    if ( integerp( a ) && integerp( b ) ) {
        struct cons_space_object *cell_a = &pointer2cell( a );
        struct cons_space_object *cell_b = &pointer2cell( b );

        result =
            cell_a->payload.integer.value == cell_b->payload.integer.value;
    }

    return result;
}

/**
 * true if `a` is an integer, and `b` is a real number whose value is the
 * value of that integer.
 */
bool equal_integer_real( struct cons_pointer a, struct cons_pointer b ) {
    bool result = false;

    if ( integerp( a ) && realp( b ) ) {
        long double bv = pointer2cell( b ).payload.real.value;

        if ( floor( bv ) == bv ) {
            result = pointer2cell( a ).payload.integer.value == ( int64_t ) bv;
        }
    }

    return result;
}
