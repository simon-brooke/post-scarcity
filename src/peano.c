/*
 * peano.c
 *
 * Basic peano arithmetic
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "equal.h"
#include "integer.h"
#include "intern.h"
#include "lispops.h"
#include "print.h"
#include "read.h"
#include "real.h"
#include "stack.h"

long double to_long_double( struct cons_pointer arg );
long int to_long_int( struct stack_frame *frame, struct cons_pointer arg );


bool zerop( struct cons_pointer arg ) {
    bool result = false;
    struct cons_space_object cell = pointer2cell( arg );

    switch ( cell.tag.value ) {
        case INTEGERTV:
            result = cell.payload.integer.value == 0;
            break;
        case RATIOTV:
            result = zerop( cell.payload.ratio.dividend );
            break;
        case REALTV:
            result = ( cell.payload.real.value == 0 );
            break;
    }

    return result;
}

/**
 * TODO: cannot throw an exception out of here, which is a problem
 * if a ratio may legally have zero as a divisor, or something which is
 * not a number is passed in.
 */
long double to_long_double( struct cons_pointer arg ) {
    long double result = NAN;   /* not a number, as a long double */
    struct cons_space_object cell = pointer2cell( arg );

    switch ( cell.tag.value ) {
        case INTEGERTV:
            result = cell.payload.integer.value * 1.0;
        case RATIOTV:
            {
                struct cons_space_object dividend =
                    pointer2cell( cell.payload.ratio.dividend );
                struct cons_space_object divisor =
                    pointer2cell( cell.payload.ratio.divisor );

                result =
                    dividend.payload.integer.value /
                    divisor.payload.integer.value;
            }
            break;
        case REALTV:
            result = cell.payload.real.value;
            break;
    }

    return result;
}


/**
 * TODO: cannot throw an exception out of here, which is a problem
 * if a ratio may legally have zero as a divisor, or something which is
 * not a number (or is a big number) is passed in.
 */
long int to_long_int( struct stack_frame *frame, struct cons_pointer arg ) {
    long int result = 0;
    struct cons_space_object cell = pointer2cell( arg );
    switch ( cell.tag.value ) {
        case INTEGERTV:
            result = cell.payload.integer.value;
            break;
        case RATIOTV:
            result = lroundl( to_long_double( arg ) );
            break;
        case REALTV:
            result = lroundl( cell.payload.real.value );
            break;
    }
    return result;
}

long int greatest_common_divisor( long int m, long int n ) {
    int o;
    while ( m ) {
        o = m;
        m = n % m;
        n = o;
    }

    return o;
}

long int least_common_multiplier( long int m, long int n ) {
    return m / greatest_common_divisor( m, n ) * n;
}

/**
* return a cons_pointer indicating a number which is the sum of
* the numbers indicated by `arg1` and `arg2`.
*/
struct cons_pointer add_2( struct stack_frame *frame, struct cons_pointer arg1,
                           struct cons_pointer arg2 ) {
    struct cons_pointer result;
    struct cons_space_object cell1 = pointer2cell( arg1 );
    struct cons_space_object cell2 = pointer2cell( arg2 );

    if ( zerop( arg1 ) ) {
        result = arg2;
    } else if ( zerop( arg2 ) ) {
        result = arg1;
    } else {

        switch ( cell1.tag.value ) {
            case EXCEPTIONTV:
                result = cell1;
                break;
            case INTEGERTV:
                switch ( cell2.tag.value ) {
                    case EXCEPTIONTV:
                        result = cell2;
                        break;
                    case INTEGERTV:
                        make_integer( cell1.payload.integer.value +
                                      cell2.payload.integer.value );
                        break;
                    case RATIOTV:
                        result = add_integer_ratio( arg1, arg2 );
                        break;
                    case REALTV:
                        result =
                            make_real( cell1.payload.integer.value +
                                       cell2.payload.real.value );
                        break;
                    default:
                        result = lisp_throw( c_string_to_lisp_string
                                             ( "Cannot add: not a number" ),
                                             frame );
                }
                break;
            case RATIOTV:
                switch ( cell2.tag.value ) {
                    case EXCEPTIONTV:
                        result = cell2;
                        break;
                    case INTEGERTV:
                        result = add_integer_ratio( arg1, arg2 );
                        break;
                    case RATIOTV:
                        break;
                    case REALTV:
                        result =
                            make_real( cell2.payload.real.value +
                                       ratio_to_long_double( arg1 ) );
                        break;
                }
        }
    }

    return result;
}

/**
 * Add an indefinite number of numbers together
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer lisp_add( struct stack_frame
                              *frame, struct
                              cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_pointer result = make_integer( 0 );
    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] ); i++ ) {
        result = add_q( frame, result, frame->arg[i] );
    }

    struct cons_pointer more = frame->more;
    while ( consp( more ) ) {
        result = add_2( frame, result, c _car( more ) );
        more = c_cdr( more );
    }

    return result;
}

/**
 * Internal guts of multiply. Dark and mysterious.
 */
struct cons_pointer multiply_accumulate( struct
                                         cons_pointer arg, struct
                                         stack_frame
                                         *frame, long
                                         int
                                         *i_accumulator, long
                                         double
                                         *d_accumulator, int
                                         *is_int ) {
    struct cons_pointer result = NIL;
    struct cons_space_object cell = pointer2cell( arg );
    switch ( cell.tag.value ) {
        case INTEGERTV:
            ( *i_accumulator ) *= cell.payload.integer.value;
            ( *d_accumulator ) *= numeric_value( arg );
            break;
        case REALTV:
            ( *d_accumulator ) *= cell.payload.real.value;
            ( *is_int ) &= false;
            break;
        case EXCEPTIONTV:
            result = arg;
            break;
        default:
            result =
                lisp_throw
                ( c_string_to_lisp_string
                  ( "Cannot multiply: not a number" ), frame );
    }
    return result;
}

/**
 * Multiply an indefinite number of numbers together
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer lisp_multiply( struct
                                   stack_frame
                                   *frame, struct
                                   cons_pointer env ) {
    struct cons_pointer result = NIL;
    long int i_accumulator = 1;
    long double d_accumulator = 1;
    int is_int = true;
    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] )
          && !exceptionp( result ); i++ ) {
        result =
            multiply_accumulate( frame->arg[i],
                                 frame,
                                 &i_accumulator, &d_accumulator, &is_int );
    }

    struct cons_pointer more = frame->more;
    while ( consp( more )
            && !exceptionp( result ) ) {
        result =
            multiply_accumulate( c_car
                                 ( more ),
                                 frame,
                                 &i_accumulator, &d_accumulator, &is_int );
        more = c_cdr( more );
    }

    if ( !exceptionp( result ) ) {
        if ( is_int ) {
            result = make_integer( i_accumulator );
        } else {
            result = make_real( d_accumulator );
        }
    }

    return result;
}

/**
 * Subtract one number from another.
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer lisp_subtract( struct
                                   stack_frame
                                   *frame, struct
                                   cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_space_object arg0 = pointer2cell( frame->arg[0] );
    struct cons_space_object arg1 = pointer2cell( frame->arg[1] );
    if ( integerp( frame->arg[0] )
         && integerp( frame->arg[1] ) ) {
        result =
            make_integer( arg0.payload.integer.value
                          - arg1.payload.integer.value );
    } else if ( realp( frame->arg[0] )
                && realp( frame->arg[1] ) ) {
        result =
            make_real( arg0.payload.real.value - arg1.payload.real.value );
    } else if ( integerp( frame->arg[0] )
                && realp( frame->arg[1] ) ) {
        result =
            make_real( numeric_value
                       ( frame->arg[0] ) - arg1.payload.real.value );
    } else if ( realp( frame->arg[0] )
                && integerp( frame->arg[1] ) ) {
        result =
            make_real( arg0.payload.real.value -
                       numeric_value( frame->arg[1] ) );
    } else {
        /* TODO: throw an exception */
        lisp_throw
            ( c_string_to_lisp_string
              ( "Cannot subtract: not a number" ), frame );
    }

    // and if not nilp[frame->arg[2]) we also have an error.

    return result;
}

/**
 * Divide one number by another.
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer lisp_divide( struct
                                 stack_frame
                                 *frame, struct
                                 cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_space_object arg0 = pointer2cell( frame->arg[0] );
    struct cons_space_object arg1 = pointer2cell( frame->arg[1] );
    if ( numberp( frame->arg[1] )
         && numeric_value( frame->arg[1] ) == 0 ) {
        lisp_throw
            ( c_string_to_lisp_string
              ( "Cannot divide: divisor is zero" ), frame );
    } else if ( numberp( frame->arg[0] )
                && numberp( frame->arg[1] ) ) {
        long int i = ( long int )
            numeric_value( frame->arg[0] ) / numeric_value( frame->arg[1] );
        long double r = ( long double )
            numeric_value( frame->arg[0] )
            / numeric_value( frame->arg[1] );
        if ( fabsl( ( long double ) i - r ) < 0.0000000001 ) {
            result = make_integer( i );
        } else {
            result = make_real( r );
        }
    } else {
        lisp_throw
            ( c_string_to_lisp_string
              ( "Cannot divide: not a number" ), frame );
    }

    return result;
}
