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

#include "conspage.h"
#include "consspaceobject.h"
#include "debug.h"
#include "equal.h"
#include "integer.h"
#include "lispops.h"
#include "peano.h"
#include "print.h"
#include "ratio.h"


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
    struct cons_space_object cell = pointer2cell( pointer );
    struct cons_space_object dividend =
        pointer2cell( cell.payload.ratio.dividend );
    struct cons_space_object divisor =
        pointer2cell( cell.payload.ratio.divisor );

    if ( divisor.payload.integer.value == 1 ) {
        result = pointer2cell( pointer ).payload.ratio.dividend;
    } else {
        if ( ratiop( pointer ) ) {
            int64_t ddrv = dividend.payload.integer.value,
                drrv = divisor.payload.integer.value,
                gcd = greatest_common_divisor( ddrv, drrv );

            if ( gcd > 1 ) {
                if ( drrv / gcd == 1 ) {
                    result = make_integer( ddrv / gcd, NIL );
                } else {
                    result =
                        make_ratio( make_integer( ddrv / gcd, NIL ),
                                    make_integer( drrv / gcd, NIL ) );
                }
            }
        }
    }

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

    debug_print( L"add_ratio_ratio( arg1 = ", DEBUG_ARITH );
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
            lcm = least_common_multiple( dr1v, dr2v ),
            m1 = lcm / dr1v, m2 = lcm / dr2v;

        debug_printf( DEBUG_ARITH, L"); lcm = %ld; m1 = %ld; m2 = %ld", lcm,
                      m1, m2 );

        if ( dr1v == dr2v ) {
            r = make_ratio( make_integer( dd1v + dd2v, NIL ),
                            cell1.payload.ratio.divisor );
        } else {
            struct cons_pointer dd1vm = make_integer( dd1v * m1, NIL ),
                dr1vm = make_integer( dr1v * m1, NIL ),
                dd2vm = make_integer( dd2v * m2, NIL ),
                dr2vm = make_integer( dr2v * m2, NIL ),
                r1 = make_ratio( dd1vm, dr1vm ),
                r2 = make_ratio( dd2vm, dr2vm );

            r = add_ratio_ratio( r1, r2 );

            /* because the references on dd1vm, dr1vm, dd2vm and dr2vm were
             * never incremented except when making r1 and r2, decrementing
             * r1 and r2 should be enought to garbage collect them. */
            dec_ref( r1 );
            dec_ref( r2 );
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

    if ( integerp( intarg ) && ratiop( ratarg ) ) {
        // TODO: not longer works
        struct cons_pointer one = make_integer( 1, NIL ),
            ratio = make_ratio( intarg, one );

        result = add_ratio_ratio( ratio, ratarg );

        dec_ref( one );
        dec_ref( ratio );
    } else {
        result =
            throw_exception( make_cons( c_string_to_lisp_string
                                        ( L"Shouldn't happen: bad arg to add_integer_ratio" ),
                                        make_cons( intarg,
                                                   make_cons( ratarg,
                                                              NIL ) ) ), NIL );
    }

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
    // TODO: this now has to work if `arg1` is an integer
    struct cons_pointer i =
        make_ratio( pointer2cell( arg2 ).payload.ratio.divisor,
                    pointer2cell( arg2 ).payload.ratio.dividend ), result =
        multiply_ratio_ratio( arg1, i );

    dec_ref( i );

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

        struct cons_pointer unsimplified =
            make_ratio( make_integer( ddrv, NIL ),
                        make_integer( drrv, NIL ) );
        result = simplify_ratio( unsimplified );

        if ( !eq( unsimplified, result ) ) {
            dec_ref( unsimplified );
        }
    } else {
        result =
            throw_exception( c_string_to_lisp_string
                             ( L"Shouldn't happen: bad arg to multiply_ratio_ratio" ),
                             NIL );
    }

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

    if ( integerp( intarg ) && ratiop( ratarg ) ) {
        // TODO: no longer works; fix
        struct cons_pointer one = make_integer( 1, NIL ),
            ratio = make_ratio( intarg, one );
        result = multiply_ratio_ratio( ratio, ratarg );

        dec_ref( one );
        dec_ref( ratio );
    } else {
        result =
            throw_exception( c_string_to_lisp_string
                             ( L"Shouldn't happen: bad arg to multiply_integer_ratio" ),
                             NIL );
    }

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
        result = allocate_cell( RATIOTV );
        struct cons_space_object *cell = &pointer2cell( result );
        cell->payload.ratio.dividend = dividend;
        cell->payload.ratio.divisor = divisor;
    } else {
        result =
            throw_exception( c_string_to_lisp_string
                             ( L"Dividend and divisor of a ratio must be integers" ),
                             NIL );
    }
    debug_dump_object( result, DEBUG_ARITH );

    return result;
}

/**
 * True if a and be are identical ratios, else false.
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
