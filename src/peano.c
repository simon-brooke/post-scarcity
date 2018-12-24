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
long int to_long_int( struct cons_pointer arg );
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
long int to_long_int( struct cons_pointer arg ) {
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

long int least_common_multiple( long int m, long int n ) {
    return m / greatest_common_divisor( m, n ) * n;
}

struct cons_pointer simplify_ratio( struct stack_frame *frame,
                                    struct cons_pointer arg ) {
    struct cons_pointer result = arg;
    long int ddrv =
        pointer2cell( pointer2cell( arg ).payload.ratio.dividend ).
        payload.integer.value, drrv =
        pointer2cell( pointer2cell( arg ).payload.ratio.divisor ).
        payload.integer.value, gcd = greatest_common_divisor( ddrv, drrv );

    if ( gcd > 1 ) {
        if ( drrv / gcd == 1 ) {
            result = make_integer( ddrv / gcd );
        } else {
            result =
                make_ratio( frame, make_integer( ddrv / gcd ),
                            make_integer( drrv / gcd ) );
        }
    }
    return result;
}


/**
 * return a cons_pointer indicating a number which is the sum of
* the ratios indicated by `arg1` and `arg2`. If you pass non-ratios,
* this is going to break horribly.
*/
struct cons_pointer add_ratio_ratio( struct stack_frame *frame,
                                     struct cons_pointer arg1,
                                     struct cons_pointer arg2 ) {
    fputws( L"add_ratio_ratio( arg1 = ", stderr );
    print( stderr, arg1 );
    fputws( L"; arg2 = ", stderr );
    print( stderr, arg2 );

    struct cons_pointer r1, result;
    struct cons_space_object cell1 = pointer2cell( arg1 );
    struct cons_space_object cell2 = pointer2cell( arg2 );
    long int dd1v =
        pointer2cell( cell1.payload.ratio.dividend ).payload.integer.value,
        dd2v =
        pointer2cell( cell2.payload.ratio.dividend ).payload.integer.value,
        dr1v =
        pointer2cell( cell1.payload.ratio.divisor ).payload.integer.value,
        dr2v =
        pointer2cell( cell2.payload.ratio.divisor ).payload.integer.value,
        lcm = least_common_multiple( dr1v, dr2v ),
        m1 = lcm / dr1v, m2 = lcm / dr2v;

    fwprintf( stderr, L"); lcm = %ld; m1 = %ld; m2 = %ld", lcm, m1, m2 );

    if ( dr1v == dr2v ) {
        r1 = make_ratio( frame,
                         make_integer( dd1v + dd2v ),
                         cell1.payload.ratio.divisor );
    } else {
        r1 = add_ratio_ratio( frame,
                              make_ratio( frame,
                                          make_integer( dd1v * m1 ),
                                          make_integer( dr1v * m1 ) ),
                              make_ratio( frame,
                                          make_integer( dd2v * m2 ),
                                          make_integer( dr2v * m2 ) ) );
    }

    result = simplify_ratio( frame, r1 );
    if ( !eq( r1, result ) ) {
        dec_ref( r1 );
    }

    fputws( L" => ", stderr );
    print( stderr, result );
    fputws( L"\n", stderr );

    return result;
}


/**
 * return a cons_pointer indicating a number which is the sum of
* the ratios indicated by `arg1` and `arg2`. If you pass non-ratios,
* this is going to break horribly.
*/
struct cons_pointer add_integer_ratio( struct stack_frame *frame,
                                       struct cons_pointer intarg,
                                       struct cons_pointer ratarg ) {
    return add_ratio_ratio( frame,
                            make_ratio( frame, intarg, make_integer( 1 ) ),
                            ratarg );
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

    fputws( L"add_2( arg1 = ", stderr );
    print( stderr, arg1 );
    fputws( L"; arg2 = ", stderr );
    print( stderr, arg2 );

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
                result = lisp_throw( c_string_to_lisp_string
                                     ( "Cannot add: not a number" ), frame );
        }
    }

    fputws( L"}; => ", stderr );
    print( stderr, arg2 );
    fputws( L"\n", stderr );

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

struct cons_pointer multiply_ratio_ratio( struct
                                          stack_frame
                                          *frame, struct
                                          cons_pointer arg1, struct
                                          cons_pointer arg2 ) {
    fputws( L"multiply_ratio_ratio( arg1 = ", stderr );
    print( stderr, arg1 );
    fputws( L"; arg2 = ", stderr );
    print( stderr, arg2 );

    struct cons_pointer result;
    struct cons_space_object cell1 = pointer2cell( arg1 );
    struct cons_space_object cell2 = pointer2cell( arg2 );
    long int dd1v =
        pointer2cell( cell1.payload.ratio.dividend ).payload.integer.value,
        dd2v =
        pointer2cell( cell2.payload.ratio.dividend ).payload.integer.value,
        dr1v =
        pointer2cell( cell1.payload.ratio.divisor ).payload.integer.value,
        dr2v =
        pointer2cell( cell2.payload.ratio.divisor ).payload.integer.value,
        ddrv = dd1v * dd2v, drrv = dr1v * dr2v;

    struct cons_pointer unsimplified = make_ratio( frame, make_integer( ddrv ),
                                                   make_integer( drrv ) );
    result = simplify_ratio( frame, unsimplified );

    if ( !eq( unsimplified, result ) ) {
        dec_ref( unsimplified );
    }

    return result;
}

/**
 * return a cons_pointer indicating a number which is the sum of
* the ratios indicated by `arg1` and `arg2`. If you pass non-ratios,
* this is going to break horribly.
*/
struct cons_pointer multiply_integer_ratio( struct stack_frame *frame,
                                            struct cons_pointer intarg,
                                            struct cons_pointer ratarg ) {
    return multiply_ratio_ratio( frame,
                                 make_ratio( frame, intarg,
                                             make_integer( 1 ) ), ratarg );
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

    fputws( L"multiply_2( arg1 = ", stderr );
    print( stderr, arg1 );
    fputws( L"; arg2 = ", stderr );
    print( stderr, arg2 );

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
    }

    fputws( L"}; => ", stderr );
    print( stderr, arg2 );
    fputws( L"\n", stderr );

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


struct cons_pointer subtract_ratio_ratio( struct stack_frame *frame,
                                          struct cons_pointer arg1,
                                          struct cons_pointer arg2 ) {
    struct cons_pointer i = inverse( frame, arg2 ),
        result = add_ratio_ratio( frame, arg1, i );

    dec_ref( i );

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
            result =
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
