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
#include "ratio.h"
#include "read.h"
#include "real.h"
#include "stack.h"

long double to_long_double( struct cons_pointer arg );
int64_t to_long_int( struct cons_pointer arg );
struct cons_pointer add_2( struct stack_frame *frame, struct cons_pointer arg1,
                           struct cons_pointer arg2 );


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
    long double result = 0;     /* not a number, as a long double */
    struct cons_space_object cell = pointer2cell( arg );

    switch ( cell.tag.value ) {
        case INTEGERTV:
            result = ( double ) cell.payload.integer.value;
            break;
        case RATIOTV:
            {
                struct cons_space_object dividend =
                    pointer2cell( cell.payload.ratio.dividend );
                struct cons_space_object divisor =
                    pointer2cell( cell.payload.ratio.divisor );

                result =
                    ( long double ) dividend.payload.integer.value /
                    divisor.payload.integer.value;
            }
            break;
        case REALTV:
            result = cell.payload.real.value;
            break;
        default:
            result = NAN;
            break;
    }

    fputws( L"to_long_double( ", stderr );
    print( stderr, arg );
    fwprintf( stderr, L") => %lf\n", result );

    return result;
}


/**
 * TODO: cannot throw an exception out of here, which is a problem
 * if a ratio may legally have zero as a divisor, or something which is
 * not a number (or is a big number) is passed in.
 */
int64_t to_long_int( struct cons_pointer arg ) {
    int64_t result = 0;
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


/**
* return a cons_pointer indicating a number which is the sum of
* the numbers indicated by `arg1` and `arg2`.
*/
struct cons_pointer add_2( struct stack_frame *frame, struct cons_pointer arg1,
                           struct cons_pointer arg2 ) {
    struct cons_pointer result;
    struct cons_space_object cell1 = pointer2cell( arg1 );
    struct cons_space_object cell2 = pointer2cell( arg2 );

#ifdef DEBUG
    fputws( L"add_2( arg1 = ", stderr );
    print( stderr, arg1 );
    fputws( L"; arg2 = ", stderr );
    print( stderr, arg2 );
    fputws( L")\n", stderr);
#endif

    if ( zerop( arg1 ) ) {
        result = arg2;
    } else if ( zerop( arg2 ) ) {
        result = arg1;
    } else {

        switch ( cell1.tag.value ) {
            case EXCEPTIONTV:
                result = arg1;
                break;
            case INTEGERTV:
                switch ( cell2.tag.value ) {
                    case EXCEPTIONTV:
                        result = arg2;
                        break;
                    case INTEGERTV:
                        result = make_integer( cell1.payload.integer.value +
                                               cell2.payload.integer.value );
                        break;
                    case RATIOTV:
                        result = add_integer_ratio( frame, arg1, arg2 );
                        break;
                    case REALTV:
                        result =
                            make_real( to_long_double( arg1 ) +
                                       to_long_double( arg2 ) );
                        break;
                    default:
                        result = lisp_throw( c_string_to_lisp_string
                                             ( "Cannot add: not a number" ),
                                             frame );
                        break;
                }
                break;
            case RATIOTV:
                switch ( cell2.tag.value ) {
                    case EXCEPTIONTV:
                        result = arg2;
                        break;
                    case INTEGERTV:
                        result = add_integer_ratio( frame, arg2, arg1 );
                        break;
                    case RATIOTV:
                        result = add_ratio_ratio( frame, arg1, arg2 );
                        break;
                    case REALTV:
                        result =
                            make_real( to_long_double( arg1 ) +
                                       to_long_double( arg2 ) );
                        break;
                    default:
                        result = lisp_throw( c_string_to_lisp_string
                                             ( "Cannot add: not a number" ),
                                             frame );
                        break;
                }
                break;
            case REALTV:
                result =
                    make_real( to_long_double( arg1 ) +
                               to_long_double( arg2 ) );
                break;
            default:
                result = exceptionp( arg2 ) ? arg2 :
                    lisp_throw( c_string_to_lisp_string
                                ( "Cannot add: not a number" ), frame );
        }
    }

#ifdef DEBUG
    fputws( L"}; => ", stderr );
    print( stderr, arg2 );
    fputws( L"\n", stderr );
#endif

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
    struct cons_pointer result = make_integer( 0 );
    struct cons_pointer tmp;

    for ( int i = 0;
          i < args_in_frame &&
          !nilp( frame->arg[i] ) && !exceptionp( result ); i++ ) {
        tmp = result;
        result = add_2( frame, result, frame->arg[i] );
        if ( !eq( tmp, result ) ) {
            dec_ref( tmp );
        }
    }

    struct cons_pointer more = frame->more;
    while ( consp( more ) && !exceptionp( result ) ) {
        tmp = result;
        result = add_2( frame, result, c_car( more ) );
        if ( !eq( tmp, result ) ) {
            dec_ref( tmp );
        }

        more = c_cdr( more );
    }

    return result;
}


/**
* return a cons_pointer indicating a number which is the product of
* the numbers indicated by `arg1` and `arg2`.
*/
struct cons_pointer multiply_2( struct stack_frame *frame,
                                struct cons_pointer arg1,
                                struct cons_pointer arg2 ) {
    struct cons_pointer result;
    struct cons_space_object cell1 = pointer2cell( arg1 );
    struct cons_space_object cell2 = pointer2cell( arg2 );

#ifdef DEBUG
    fputws( L"multiply_2( arg1 = ", stderr );
    print( stderr, arg1 );
    fputws( L"; arg2 = ", stderr );
    print( stderr, arg2 );
    fputws( L")\n", stderr);
#endif

    if ( zerop( arg1 ) ) {
        result = arg2;
    } else if ( zerop( arg2 ) ) {
        result = arg1;
    } else {
        switch ( cell1.tag.value ) {
            case EXCEPTIONTV:
                result = arg1;
                break;
            case INTEGERTV:
                switch ( cell2.tag.value ) {
                    case EXCEPTIONTV:
                        result = arg2;
                        break;
                    case INTEGERTV:
                        result = make_integer( cell1.payload.integer.value *
                                               cell2.payload.integer.value );
                        break;
                    case RATIOTV:
                        result = multiply_integer_ratio( frame, arg1, arg2 );
                        break;
                    case REALTV:
                        result =
                            make_real( to_long_double( arg1 ) *
                                       to_long_double( arg2 ) );
                        break;
                    default:
                        result = lisp_throw( c_string_to_lisp_string
                                             ( "Cannot multiply: not a number" ),
                                             frame );
                        break;
                }
                break;
            case RATIOTV:
                switch ( cell2.tag.value ) {
                    case EXCEPTIONTV:
                        result = arg2;
                        break;
                    case INTEGERTV:
                        result = multiply_integer_ratio( frame, arg2, arg1 );
                        break;
                    case RATIOTV:
                        result = multiply_ratio_ratio( frame, arg1, arg2 );
                        break;
                    case REALTV:
                        result =
                            make_real( to_long_double( arg1 ) *
                                       to_long_double( arg2 ) );
                        break;
                    default:
                        result = lisp_throw( c_string_to_lisp_string
                                             ( "Cannot multiply: not a number" ),
                                             frame );
                }
                break;
            case REALTV:
                result = exceptionp( arg2 ) ? arg2 :
                    make_real( to_long_double( arg1 ) *
                               to_long_double( arg2 ) );
                break;
            default:
                result = lisp_throw( c_string_to_lisp_string
                                     ( "Cannot multiply: not a number" ),
                                     frame );
                break;
        }
    }

#ifdef DEBUG
    fputws( L" => ", stderr );
    print( stderr, arg2 );
    fputws( L"\n", stderr );
#endif

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
    struct cons_pointer result = make_integer( 1 );
    struct cons_pointer tmp;

    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] )
          && !exceptionp( result ); i++ ) {
        tmp = result;
        result = multiply_2( frame, result, frame->arg[i] );

        if ( !eq( tmp, result ) ) {
            dec_ref( tmp );
        }
    }

    struct cons_pointer more = frame->more;
    while ( consp( more )
            && !exceptionp( result ) ) {
        tmp = result;
        result = multiply_2( frame, result, c_car( more ) );

        if ( !eq( tmp, result ) ) {
            dec_ref( tmp );
        }

        more = c_cdr( more );
    }

    return result;
}

/**
 * return a cons_pointer indicating a number which is the
 * inverse of the number indicated by `arg`.
 */
struct cons_pointer inverse( struct stack_frame *frame,
                             struct cons_pointer arg ) {
    struct cons_pointer result = NIL;
    struct cons_space_object cell = pointer2cell( arg );

    switch ( cell.tag.value ) {
        case EXCEPTIONTV:
            result = arg;
            break;
        case INTEGERTV:
            result = make_integer( 0 - to_long_int( arg ) );
            break;
        case NILTV:
            result = TRUE;
            break;
        case RATIOTV:
            result = make_ratio( frame,
                                 make_integer( 0 -
                                               to_long_int( cell.payload.
                                                            ratio.dividend ) ),
                                 cell.payload.ratio.divisor );
            break;
        case REALTV:
            result = make_real( 0 - to_long_double( arg ) );
            break;
        case TRUETV:
            result = NIL;
            break;
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
    struct cons_space_object cell0 = pointer2cell( frame->arg[0] );
    struct cons_space_object cell1 = pointer2cell( frame->arg[1] );

    switch ( cell0.tag.value ) {
        case EXCEPTIONTV:
            result = frame->arg[0];
            break;
        case INTEGERTV:
            switch ( cell1.tag.value ) {
                case EXCEPTIONTV:
                    result = frame->arg[1];
                    break;
                case INTEGERTV:
                    result = make_integer( cell0.payload.integer.value
                                           - cell1.payload.integer.value );
                    break;
                case RATIOTV:{
                        struct cons_pointer tmp =
                            make_ratio( frame, frame->arg[0],
                                        make_integer( 1 ) );
                        result =
                            subtract_ratio_ratio( frame, tmp, frame->arg[1] );
                        dec_ref( tmp );
                    }
                    break;
                case REALTV:
                    result =
                        make_real( to_long_double( frame->arg[0] ) -
                                   to_long_double( frame->arg[1] ) );
                    break;
                default:
                    result = lisp_throw( c_string_to_lisp_string
                                         ( "Cannot multiply: not a number" ),
                                         frame );
                    break;
            }
            break;
        case RATIOTV:
            switch ( cell1.tag.value ) {
                case EXCEPTIONTV:
                    result = frame->arg[1];
                    break;
                case INTEGERTV:{
                        struct cons_pointer tmp =
                            make_ratio( frame, frame->arg[1],
                                        make_integer( 1 ) );
                        result =
                            subtract_ratio_ratio( frame, frame->arg[0], tmp );
                        dec_ref( tmp );
                    }
                    break;
                case RATIOTV:
                    result =
                        subtract_ratio_ratio( frame, frame->arg[0],
                                              frame->arg[1] );
                    break;
                case REALTV:
                    result =
                        make_real( to_long_double( frame->arg[0] ) -
                                   to_long_double( frame->arg[1] ) );
                    break;
                default:
                    result = lisp_throw( c_string_to_lisp_string
                                         ( "Cannot multiply: not a number" ),
                                         frame );
                    break;
            }
            break;
        case REALTV:
            result = exceptionp( frame->arg[1] ) ? frame->arg[1] :
                make_real( to_long_double( frame->arg[0] ) -
                           to_long_double( frame->arg[1] ) );
            break;
        default:
            result = lisp_throw( c_string_to_lisp_string
                                 ( "Cannot multiply: not a number" ), frame );
            break;
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

    switch ( arg0.tag.value ) {
        case EXCEPTIONTV:
            result = frame->arg[0];
            break;
        case INTEGERTV:
            switch ( arg1.tag.value ) {
                case EXCEPTIONTV:
                    result = frame->arg[1];
                    break;
                case INTEGERTV:{
                        struct cons_pointer unsimplified =
                            make_ratio( frame, frame->arg[0], frame->arg[1] );
                        result = simplify_ratio( frame, unsimplified );
                        if ( !eq( unsimplified, result ) ) {
                            dec_ref( unsimplified );
                        }
                    }
                    break;
                case RATIOTV:{
                        struct cons_pointer one = make_integer( 1 );
                        struct cons_pointer ratio =
                            make_ratio( frame, frame->arg[0], one );
                        result =
                            divide_ratio_ratio( frame, ratio, frame->arg[1] );
                        dec_ref( ratio );
                    }
                    break;
                case REALTV:
                    result =
                        make_real( to_long_double( frame->arg[0] ) /
                                   to_long_double( frame->arg[1] ) );
                    break;
                default:
                    result = lisp_throw( c_string_to_lisp_string
                                         ( "Cannot divide: not a number" ),
                                         frame );
                    break;
            }
            break;
        case RATIOTV:
            switch ( arg1.tag.value ) {
                case EXCEPTIONTV:
                    result = frame->arg[1];
                    break;
                case INTEGERTV:{
                        struct cons_pointer one = make_integer( 1 );
                        struct cons_pointer ratio =
                            make_ratio( frame, frame->arg[1], one );
                        result =
                            divide_ratio_ratio( frame, frame->arg[0], ratio );
                        dec_ref( ratio );
                    }
                    break;
                case RATIOTV:
                    result =
                        divide_ratio_ratio( frame, frame->arg[0],
                                            frame->arg[1] );
                    break;
                case REALTV:
                    result =
                        make_real( to_long_double( frame->arg[0] ) /
                                   to_long_double( frame->arg[1] ) );
                    break;
                default:
                    result = lisp_throw( c_string_to_lisp_string
                                         ( "Cannot divide: not a number" ),
                                         frame );
                    break;
            }
            break;
        case REALTV:
            result = exceptionp( frame->arg[1] ) ? frame->arg[1] :
                make_real( to_long_double( frame->arg[0] ) /
                           to_long_double( frame->arg[1] ) );
            break;
        default:
            result = lisp_throw( c_string_to_lisp_string
                                 ( "Cannot divide: not a number" ), frame );
            break;
    }

    return result;
}
