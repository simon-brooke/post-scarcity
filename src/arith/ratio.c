/*
 * ratio.c
 *
 * functions for rational number cells.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>

#include "arith/integer.h"
#include "arith/peano.h"
#include "arith/ratio.h"
#include "arith/real.h"
#include "debug.h"
#include "io/print.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/stack.h"
#include "ops/equal.h"
#include "ops/lispops.h"


/**
 * return, as a int64_t, the greatest common divisor of `m` and `n`,
 */
int64_t greatest_common_divisor( int64_t m, int64_t n ) {
    int o;
    while ( m ) {
        o = m;
        m = n % m;
        n = o;
    }

    return o;
}

/**
 * return, as a int64_t, the least common multiple of `m` and `n`,
 */
int64_t least_common_multiple( int64_t m, int64_t n ) {
    return m / greatest_common_divisor( m, n ) * n;
}

struct cons_pointer simplify_ratio( struct cons_pointer pointer ) {
    struct cons_pointer result = pointer;

    if ( ratiop( pointer ) ) {
        struct cons_space_object cell = pointer2cell( pointer );
        struct cons_space_object dividend =
            pointer2cell( cell.payload.ratio.dividend );
        struct cons_space_object divisor =
            pointer2cell( cell.payload.ratio.divisor );

        if ( divisor.payload.integer.value == 1 ) {
            result = pointer2cell( pointer ).payload.ratio.dividend;
        } else {
            int64_t ddrv = dividend.payload.integer.value,
                drrv = divisor.payload.integer.value,
                gcd = greatest_common_divisor( ddrv, drrv );

            if ( gcd > 1 ) {
                if ( drrv / gcd == 1 ) {
                    result = acquire_integer( ddrv / gcd, NIL );
                } else {
                    debug_printf( DEBUG_ARITH,
                                  L"simplify_ratio: %ld/%ld => %ld/%ld\n",
                                  ddrv, drrv, ddrv / gcd, drrv / gcd );
                    result =
                        make_ratio( acquire_integer( ddrv / gcd, NIL ),
                                    acquire_integer( drrv / gcd, NIL ) );
                }
            }
        }
    }
    // TODO: else throw exception?

    return result;

}


/**
 * return a cons_pointer indicating a number which is the sum of
 * the ratios indicated by `arg1` and `arg2`.
 * @exception will return an exception if either `arg1` or `arg2` is not a
 * rational number.
 */
struct cons_pointer add_ratio_ratio( struct cons_pointer arg1,
                                     struct cons_pointer arg2 ) {
    struct cons_pointer r, result;

    debug_print( L"\naadd_ratio_ratio: ", DEBUG_ARITH);
    debug_print_object( arg1, DEBUG_ARITH);
    debug_print( L" + ", DEBUG_ARITH);
    debug_print_object( arg2, DEBUG_ARITH);

    if ( ratiop( arg1 ) && ratiop( arg2 ) ) {
        struct cons_space_object cell1 = pointer2cell( arg1 );
        struct cons_space_object cell2 = pointer2cell( arg2 );
        int64_t dd1v =
            pointer2cell( cell1.payload.ratio.dividend ).payload.integer.value,
            dd2v =
            pointer2cell( cell2.payload.ratio.dividend ).payload.integer.value,
            dr1v =
            pointer2cell( cell1.payload.ratio.divisor ).payload.integer.value,
            dr2v =
            pointer2cell( cell2.payload.ratio.divisor ).payload.integer.value,
            lcm = least_common_multiple( dr1v, dr2v ),
            m1 = lcm / dr1v, m2 = lcm / dr2v;

        debug_printf( DEBUG_ARITH, L"; lcm = %ld; m1 = %ld; m2 = %ld", lcm,
                      m1, m2 );

        if ( dr1v == dr2v ) {
            r = make_ratio( acquire_integer( dd1v + dd2v, NIL ),
                            cell1.payload.ratio.divisor );
        } else {
            struct cons_pointer dd1vm = acquire_integer( dd1v * m1, NIL ),
                dr1vm = acquire_integer( dr1v * m1, NIL ),
                dd2vm = acquire_integer( dd2v * m2, NIL ),
                dr2vm = acquire_integer( dr2v * m2, NIL ),
                r1 = make_ratio( dd1vm, dr1vm ),
                r2 = make_ratio( dd2vm, dr2vm );

            r = add_ratio_ratio( r1, r2 );

            if ( !eq( r, r1 ) ) {
                dec_ref( r1 );
            }
            if ( !eq( r, r2 ) ) {
                dec_ref( r2 );
            }

            /* because the references on dd1vm, dr1vm, dd2vm and dr2vm were
             * never incremented except when making r1 and r2, decrementing
             * r1 and r2 should be enought to garbage collect them. */
        }

        result = simplify_ratio( r );
        if ( !eq( r, result ) ) {
            dec_ref( r );
        }
    } else {
        result =
            throw_exception( make_cons( c_string_to_lisp_string
                                        ( L"Shouldn't happen: bad arg to add_ratio_ratio" ),
                                        make_cons( arg1,
                                                   make_cons( arg2, NIL ) ) ),
                             NIL );
    }

    debug_print( L" => ", DEBUG_ARITH );
    debug_print_object( result, DEBUG_ARITH );
    debug_print( L"\n", DEBUG_ARITH );

    return result;
}


/**
 * return a cons_pointer indicating a number which is the sum of
 * the intger indicated by `intarg` and the ratio indicated by
 * `ratarg`.
 * @exception if either `intarg` or `ratarg` is not of the expected type.
 */
struct cons_pointer add_integer_ratio( struct cons_pointer intarg,
                                       struct cons_pointer ratarg ) {
    struct cons_pointer result;

    debug_print( L"\nadd_integer_ratio: ", DEBUG_ARITH);
    debug_print_object( intarg, DEBUG_ARITH);
    debug_print( L" + ", DEBUG_ARITH);
    debug_print_object( ratarg, DEBUG_ARITH);

    if ( integerp( intarg ) && ratiop( ratarg ) ) {
        // TODO: not longer works
        struct cons_pointer one = acquire_integer( 1, NIL ),
            ratio = make_ratio( intarg, one );

        result = add_ratio_ratio( ratio, ratarg );

        release_integer( one );
        dec_ref( ratio );
    } else {
        result =
            throw_exception( make_cons( c_string_to_lisp_string
                                        ( L"Shouldn't happen: bad arg to add_integer_ratio" ),
                                        make_cons( intarg,
                                                   make_cons( ratarg,
                                                              NIL ) ) ), NIL );
    }

    debug_print( L" => ", DEBUG_ARITH );
    debug_print_object( result, DEBUG_ARITH );
    debug_print( L"\n", DEBUG_ARITH );

    return result;
}

/**
 * return a cons_pointer to a ratio which represents the value of the ratio
 * indicated by `arg1` divided by the ratio indicated by `arg2`.
 * @exception will return an exception if either `arg1` or `arg2` is not a
 * rational number.
 */
struct cons_pointer divide_ratio_ratio( struct cons_pointer arg1,
                                        struct cons_pointer arg2 ) {
    debug_print( L"\ndivide_ratio_ratio: ", DEBUG_ARITH);
    debug_print_object( arg1, DEBUG_ARITH);
    debug_print( L" / ", DEBUG_ARITH);
    debug_print_object( arg2, DEBUG_ARITH);
    // TODO: this now has to work if `arg1` is an integer
    struct cons_pointer i =
        make_ratio( pointer2cell( arg2 ).payload.ratio.divisor,
                    pointer2cell( arg2 ).payload.ratio.dividend ), result =
        multiply_ratio_ratio( arg1, i );

    dec_ref( i );

    debug_print( L" => ", DEBUG_ARITH );
    debug_print_object( result, DEBUG_ARITH );
    debug_print( L"\n", DEBUG_ARITH );

    return result;
}

/**
 * return a cons_pointer indicating a number which is the product of
 * the ratios indicated by `arg1` and `arg2`.
 * @exception will return an exception if either `arg1` or `arg2` is not a
 * rational number.
 */
struct cons_pointer multiply_ratio_ratio( struct
                                          cons_pointer arg1, struct
                                          cons_pointer arg2 ) {
    // TODO: this now has to work if arg1 is an integer
    struct cons_pointer result;

    debug_print( L"multiply_ratio_ratio( arg1 = ", DEBUG_ARITH );
    debug_print_object( arg1, DEBUG_ARITH );
    debug_print( L"; arg2 = ", DEBUG_ARITH );
    debug_print_object( arg2, DEBUG_ARITH );
    debug_print( L")\n", DEBUG_ARITH );

    if ( ratiop( arg1 ) && ratiop( arg2 ) ) {
        struct cons_space_object cell1 = pointer2cell( arg1 );
        struct cons_space_object cell2 = pointer2cell( arg2 );
        int64_t dd1v =
            pointer2cell( cell1.payload.ratio.dividend ).payload.integer.value,
            dd2v =
            pointer2cell( cell2.payload.ratio.dividend ).payload.integer.value,
            dr1v =
            pointer2cell( cell1.payload.ratio.divisor ).payload.integer.value,
            dr2v =
            pointer2cell( cell2.payload.ratio.divisor ).payload.integer.value,
            ddrv = dd1v * dd2v, drrv = dr1v * dr2v;

        struct cons_pointer dividend = acquire_integer( ddrv, NIL );
        struct cons_pointer divisor = acquire_integer( drrv, NIL );
        struct cons_pointer unsimplified = make_ratio( dividend, divisor );
        result = simplify_ratio( unsimplified );

        release_integer( dividend );
        release_integer( divisor );

        if ( !eq( unsimplified, result ) ) {
            dec_ref( unsimplified );
        }
    } else {
        result =
            throw_exception( c_string_to_lisp_string
                             ( L"Shouldn't happen: bad arg to multiply_ratio_ratio" ),
                             NIL );
    }

    debug_print( L" => ", DEBUG_ARITH );
    debug_print_object( result, DEBUG_ARITH );
    debug_print( L"\n", DEBUG_ARITH );

    return result;
}

/**
 * return a cons_pointer indicating a number which is the product of
 * the intger indicated by `intarg` and the ratio indicated by
 * `ratarg`.
 * @exception if either `intarg` or `ratarg` is not of the expected type.
 */
struct cons_pointer multiply_integer_ratio( struct cons_pointer intarg,
                                            struct cons_pointer ratarg ) {
    struct cons_pointer result;

    debug_print( L"\nmultiply_integer_ratio: ", DEBUG_ARITH);
    debug_print_object( intarg, DEBUG_ARITH);
    debug_print( L" * ", DEBUG_ARITH);
    debug_print_object( ratarg, DEBUG_ARITH);

    if ( integerp( intarg ) && ratiop( ratarg ) ) {
        // TODO: no longer works; fix
        struct cons_pointer one = acquire_integer( 1, NIL ),
            ratio = make_ratio( intarg, one );
        result = multiply_ratio_ratio( ratio, ratarg );

        release_integer( one );
    } else {
        result =
            throw_exception( c_string_to_lisp_string
                             ( L"Shouldn't happen: bad arg to multiply_integer_ratio" ),
                             NIL );
    }

    debug_print( L" => ", DEBUG_ARITH );
    debug_print_object( result, DEBUG_ARITH );
    debug_print( L"\n", DEBUG_ARITH );

    return result;
}


/**
 * return a cons_pointer indicating a number which is the difference of
 * the ratios indicated by `arg1` and `arg2`.
 * @exception will return an exception if either `arg1` or `arg2` is not a
 * rational number.
 */
struct cons_pointer subtract_ratio_ratio( struct cons_pointer arg1,
                                          struct cons_pointer arg2 ) {
    debug_print( L"\nsubtract_ratio_ratio: ", DEBUG_ARITH);
    debug_print_object( arg1, DEBUG_ARITH);
    debug_print( L" * ", DEBUG_ARITH);
    debug_print_object( arg2, DEBUG_ARITH);

    struct cons_pointer i = negative( arg2 ),
        result = add_ratio_ratio( arg1, i );

    dec_ref( i );

    return result;
}


/**
 * Construct a ratio frame from this `dividend` and `divisor`, expected to
 * be integers, in the context of the stack_frame indicated by this
 * `frame_pointer`.
 * @exception if either `dividend` or `divisor` is not an integer.
 */
struct cons_pointer make_ratio( struct cons_pointer dividend,
                                struct cons_pointer divisor ) {
    struct cons_pointer result;
    if ( integerp( dividend ) && integerp( divisor ) ) {
        inc_ref( dividend );
        inc_ref( divisor );
        struct cons_pointer unsimplified = allocate_cell( RATIOTV );
        struct cons_space_object *cell = &pointer2cell( unsimplified );
        cell->payload.ratio.dividend = dividend;
        cell->payload.ratio.divisor = divisor;

        result = simplify_ratio( unsimplified );
        if ( !eq( result, unsimplified ) ) {
            dec_ref( unsimplified );
        }
    } else {
        result =
            throw_exception( c_string_to_lisp_string
                             ( L"Dividend and divisor of a ratio must be integers" ),
                             NIL );
    }
    // debug_print( L"make_ratio returning:\n", DEBUG_ARITH);
    debug_dump_object( result, DEBUG_ARITH );

    return result;
}

/**
 * True if a and be are identical rationals, else false.
 *
 * TODO: we need ways of checking whether rationals are equal
 * to floats and to integers.
 */
bool equal_ratio_ratio( struct cons_pointer a, struct cons_pointer b ) {
    bool result = false;

    if ( ratiop( a ) && ratiop( b ) ) {
        struct cons_space_object *cell_a = &pointer2cell( a );
        struct cons_space_object *cell_b = &pointer2cell( b );

        result = equal_integer_integer( cell_a->payload.ratio.dividend,
                                        cell_b->payload.ratio.dividend ) &&
            equal_integer_integer( cell_a->payload.ratio.divisor,
                                   cell_b->payload.ratio.divisor );
    }

    return result;
}

/**
 * @brief convert a ratio to an equivalent long double.
 * 
 * @param rat a pointer to a ratio.
 * @return long double 
 */
long double c_ratio_to_ld( struct cons_pointer rat) {
    long double result = NAN;

    debug_print( L"\nc_ratio_to_ld: ", DEBUG_ARITH);
    debug_print_object( rat, DEBUG_ARITH);

    if ( ratiop( rat)) {
        struct cons_space_object * cell_a = & pointer2cell( rat);
        struct cons_pointer dv = cell_a->payload.ratio.divisor;
        struct cons_space_object * dv_cell = &pointer2cell( dv);
        struct cons_pointer dd = cell_a->payload.ratio.dividend;
        struct cons_space_object * dd_cell = &pointer2cell( dd);
        
        if ( nilp( dv_cell->payload.integer.more) && nilp( dd_cell->payload.integer.more)) {
            result = ((long double) dd_cell->payload.integer.value) / ((long double) dv_cell->payload.integer.value);;
        } else {
            fwprintf( stderr, L"real conversion is not yet implemented for bignums rationals.");
        } 
    }

    debug_printf( DEBUG_ARITH, L"\nc_ratio_to_ld returning %d\n", result );

    return result;
}

