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
#include "equal.h"
#include "lispops.h"
#include "peano.h"

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
    long int val = nilp( c ) ?
      0 :
      pointer2cell( c ).payload.integer.value;

    long int carry = is_first_cell ? 0 : ( MAX_INTEGER + 1 );

    __int128_t result = ( __int128_t ) integerp( c ) ?
        ( val == 0 ) ?
          carry :
          val :
         op == '*' ? 1 : 0;
    debug_printf( DEBUG_ARITH,
                  L"cell_value: raw value is %ld, is_first_cell = %s; %4.4s; returning ",
                  val, is_first_cell ? "true" : "false", pointer2cell(c).tag.bytes);
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
                                      struct cons_pointer new)
{
    struct cons_pointer cursor = NIL;
    __int128_t carry = 0;

    if ( MAX_INTEGER >= val ) {
        carry = 0;
    } else {
        carry = val >> 60;
        debug_printf( DEBUG_ARITH,
                     L"int128_to_integer: 64 bit overflow; setting carry to %ld\n",
                     ( int64_t ) carry );
        val &= MAX_INTEGER;
    }

    struct cons_space_object * newc = &pointer2cell( new);
    newc->payload.integer.value = val;

    if ( integerp( less_significant ) ) {
        struct cons_space_object *lsc = &pointer2cell( less_significant );
        inc_ref( new );
        lsc->payload.integer.more = new;
    }

    return carry;
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
    debug_print_object(a, DEBUG_ARITH);
    debug_print( L"; b = ", DEBUG_ARITH );
    debug_print_object(b, DEBUG_ARITH);
    debug_println(DEBUG_ARITH);

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

            struct cons_pointer new = make_integer( 0, NIL);
            carry = int128_to_integer(rv, cursor, new);
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

struct cons_pointer base_partial(int depth) {
  struct cons_pointer result = NIL;

  for (int i = 0; i < depth; i++) {
    result = make_integer(0, result);
  }

  return result;
}

/**
 * Return a pointer to an integer representing the product of the integers
 * pointed to by `a` and `b`. If either isn't an integer, will return nil.
 * \todo it is MUCH more complicated than this!
 *
 * @param a an integer;
 * @param b an integer.
 */
struct cons_pointer multiply_integers( struct cons_pointer a,
                                       struct cons_pointer b ) {
    struct cons_pointer result = NIL;
    bool neg = is_negative(a) != is_negative(b);
    bool is_first_b = true;
    int oom = 0;

    debug_print( L"multiply_integers: a = ", DEBUG_ARITH );
    debug_print_object(a, DEBUG_ARITH);
    debug_print( L"; b = ", DEBUG_ARITH );
    debug_print_object(b, DEBUG_ARITH);
    debug_println(DEBUG_ARITH);

    if ( integerp( a ) && integerp( b ) ) {
        while ( !nilp( b ) ) {
          bool is_first_d = true;
          struct cons_pointer d = a;
          struct cons_pointer partial = base_partial(oom++);
          __int128_t carry = 0;

          while ( !nilp(d) || carry != 0) {
            partial = make_integer(0, partial);
            struct cons_pointer new = make_integer( 0, NIL);
            __int128_t dv = cell_value( d, '*', is_first_d );
            __int128_t bv = cell_value( b, '*', is_first_b );

            __int128_t rv = (dv * bv) + carry;

            debug_print( L"multiply_integers: d = ", DEBUG_ARITH);
            debug_print_object( d, DEBUG_ARITH);
            debug_print( L"; dv = ", DEBUG_ARITH );
            debug_print_128bit( dv, DEBUG_ARITH );
            debug_print( L"; bv = ", DEBUG_ARITH );
            debug_print_128bit( bv, DEBUG_ARITH );
            debug_print( L"; carry = ", DEBUG_ARITH );
            debug_print_128bit( carry, DEBUG_ARITH );
            debug_print( L"; rv = ", DEBUG_ARITH );
            debug_print_128bit( rv, DEBUG_ARITH );
            debug_print( L"; acc = ", DEBUG_ARITH );
            debug_print_object( result, DEBUG_ARITH);
            debug_print( L"; partial = ", DEBUG_ARITH );
            debug_print_object( partial, DEBUG_ARITH);
            debug_print( L"\n", DEBUG_ARITH );

            inc_ref(new);
            carry = int128_to_integer(rv, NIL, new);

            if (nilp(d) && carry != 0) debug_print(L"THIS SHOULD NEVER HAPPEN!\n", DEBUG_ARITH);

            if (nilp(partial) || zerop(partial)) {
              partial = new;
            } else {
              partial = add_integers(partial, new);
              inc_ref(partial);
              //dec_ref(new);
            }

            d = integerp(d) ? pointer2cell( d ).payload.integer.more : NIL;
            is_first_d = false;
          }

          if (nilp(result) || zerop(result)) {
            result = partial;
          } else {
            struct cons_pointer old = result;
            result = add_integers(partial, result);
            //if (!eq(result, old)) dec_ref(old);
            //if (!eq(result, partial)) dec_ref(partial);
          }
          b = pointer2cell( b ).payload.integer.more;
          is_first_b = false;
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
            //dec_ref( tmp );
        }

        if ( is_negative ) {
            result = make_string( L'-', result );
        }
    }

    return result;
}
