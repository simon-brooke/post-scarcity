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

/**
 * Internal guts of add. Dark and mysterious.
 */
struct cons_pointer add_accumulate( struct cons_pointer arg,
                                    struct stack_frame *frame,
                                    long int *i_accumulator,
                                    long double *d_accumulator, int *is_int ) {
    struct cons_pointer result = NIL;
    struct cons_space_object cell = pointer2cell( arg );

    switch ( cell.tag.value ) {
        case INTEGERTV:
            ( *i_accumulator ) += cell.payload.integer.value;
            ( *d_accumulator ) += numeric_value( arg );
            break;
        case REALTV:
            ( *d_accumulator ) += cell.payload.real.value;
            ( *is_int ) &= false;
            break;
        case EXCEPTIONTV:
            result = arg;
            break;
        default:
            result = lisp_throw( c_string_to_lisp_string
                                 ( "Cannot multiply: not a number" ), frame );
    }
    return result;
}


/**
 * Add an indefinite number of numbers together
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 */
struct cons_pointer
lisp_add( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    long int i_accumulator = 0;
    long double d_accumulator = 0;
    int is_int = true;

    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] ); i++ ) {
        result =
            add_accumulate( frame->arg[i], frame, &i_accumulator,
                            &d_accumulator, &is_int );
    }

    struct cons_pointer more = frame->more;

    while ( consp( more ) ) {
        result =
            add_accumulate( c_car( more ), frame, &i_accumulator,
                            &d_accumulator, &is_int );
        more = c_cdr( more );
    }

    if ( is_int ) {
        result = make_integer( i_accumulator );
    } else {
        result = make_real( d_accumulator );
    }

    return result;
}

/**
 * Internal guts of multiply. Dark and mysterious.
 */
struct cons_pointer multiply_accumulate( struct cons_pointer arg,
                                         struct stack_frame *frame,
                                         long int *i_accumulator,
                                         long double *d_accumulator,
                                         int *is_int ) {
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
            result = lisp_throw( c_string_to_lisp_string
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
struct cons_pointer
lisp_multiply( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    long int i_accumulator = 1;
    long double d_accumulator = 1;
    int is_int = true;

    for ( int i = 0;
          i < args_in_frame && !nilp( frame->arg[i] ) && !exceptionp( result );
          i++ ) {
        result =
            multiply_accumulate( frame->arg[i], frame, &i_accumulator,
                                 &d_accumulator, &is_int );
    }

    struct cons_pointer more = frame->more;

    while ( consp( more ) && !exceptionp( result ) ) {
        result =
            multiply_accumulate( c_car( more ), frame, &i_accumulator,
                                 &d_accumulator, &is_int );
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
struct cons_pointer
lisp_subtract( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    struct cons_space_object arg0 = pointer2cell( frame->arg[0] );
    struct cons_space_object arg1 = pointer2cell( frame->arg[1] );

    if ( integerp( frame->arg[0] ) && integerp( frame->arg[1] ) ) {
        result =
            make_integer( arg0.payload.integer.value -
                          arg1.payload.integer.value );
    } else if ( realp( frame->arg[0] ) && realp( frame->arg[1] ) ) {
        result =
            make_real( arg0.payload.real.value - arg1.payload.real.value );
    } else if ( integerp( frame->arg[0] ) && realp( frame->arg[1] ) ) {
        result =
            make_real( numeric_value( frame->arg[0] ) -
                       arg1.payload.real.value );
    } else if ( realp( frame->arg[0] ) && integerp( frame->arg[1] ) ) {
        result =
            make_real( arg0.payload.real.value -
                       numeric_value( frame->arg[1] ) );
    } else {
        /* TODO: throw an exception */
        lisp_throw( c_string_to_lisp_string
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
struct cons_pointer
lisp_divide( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    struct cons_space_object arg0 = pointer2cell( frame->arg[0] );
    struct cons_space_object arg1 = pointer2cell( frame->arg[1] );

    if ( numberp( frame->arg[1] ) && numeric_value( frame->arg[1] ) == 0 ) {
        lisp_throw( c_string_to_lisp_string
                    ( "Cannot divide: divisor is zero" ), frame );
    } else if ( numberp( frame->arg[0] ) && numberp( frame->arg[1] ) ) {
        long int i = ( long int ) numeric_value( frame->arg[0] ) /
            numeric_value( frame->arg[1] );
        long double r = ( long double ) numeric_value( frame->arg[0] ) /
            numeric_value( frame->arg[1] );
        if ( fabsl( ( long double ) i - r ) < 0.0000000001 ) {
            result = make_integer( i );
        } else {
            result = make_real( r );
        }
    } else {
        lisp_throw( c_string_to_lisp_string
                    ( "Cannot divide: not a number" ), frame );
    }

    return result;
}
