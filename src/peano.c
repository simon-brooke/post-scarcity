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
    bool is_int = true;

    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] ); i++ ) {
        struct cons_space_object current = pointer2cell( frame->arg[i] );

        switch ( current.tag.value ) {
        case INTEGERTV:
            i_accumulator += current.payload.integer.value;
            d_accumulator += numeric_value( frame->arg[i] );
            break;
        case REALTV:
            d_accumulator += current.payload.real.value;
            is_int = false;
            break;
        default:
            lisp_throw( c_string_to_lisp_string( "Cannot add: not a number" ),
                        frame );
        }

        if ( !nilp( frame->more ) ) {
            lisp_throw( c_string_to_lisp_string
                        ( "Cannot yet add more than 8 numbers" ), frame );
        }

        if ( is_int ) {
            result = make_integer( i_accumulator );
        } else {
            result = make_real( d_accumulator );
        }
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
    bool is_int = true;

    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] ); i++ ) {
        struct cons_space_object arg = pointer2cell( frame->arg[i] );

        switch ( arg.tag.value ) {
        case INTEGERTV:
            i_accumulator *= arg.payload.integer.value;
            d_accumulator *= numeric_value( frame->arg[i] );
            break;
        case REALTV:
            d_accumulator *= arg.payload.real.value;
            is_int = false;
            break;
        default:
            lisp_throw( c_string_to_lisp_string
                        ( "Cannot multiply: not a number" ), frame );
        }

        if ( !nilp( frame->more ) ) {
            lisp_throw( c_string_to_lisp_string
                        ( "Cannot yet multiply more than 8 numbers" ), frame );
        }

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

  if ( numberp(frame->arg[1]) && numeric_value(frame->arg[1]) == 0) {
    lisp_throw( c_string_to_lisp_string
                        ( "Cannot divide: divisor is zero" ), frame );
  } else if ( integerp( frame->arg[0] ) && integerp( frame->arg[1] ) ) {
      result = make_integer( arg0.payload.integer.value /
                          arg1.payload.integer.value );
    } else if ( numberp(frame->arg[0]) && numberp(frame->arg[1])) {
      result = make_real( numeric_value(frame->arg[0]) / numeric_value(frame->arg[1]));
    } else {
            lisp_throw( c_string_to_lisp_string
                        ( "Cannot divide: not a number" ), frame );
    }

  return result;
}



