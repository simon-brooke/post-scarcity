/*
 * peano.c
 *
 * Basic peano arithmetic
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "debug.h"
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
struct cons_pointer add_2( struct stack_frame *frame,
                           struct cons_pointer frame_pointer,
                           struct cons_pointer arg1,
                           struct cons_pointer arg2 );

/**
 * return true if this `arg` points to a number whose value is zero.
 */
bool zerop( struct cons_pointer arg ) {
    bool result = false;
    struct cons_space_object cell = pointer2cell( arg );

    switch ( cell.tag.value ) {
        case INTEGERTV:
            result = cell.payload.integer.value == 0 &&
                nilp( cell.payload.integer.more );
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
 * Return the closest possible `binary64` representation to the value of
 * this `arg`, expected to be an integer, ratio or real, or `NAN` if `arg`
 * is not any of these.
 *
 * @arg a pointer to an integer, ratio or real.
 *
 * \todo cannot throw an exception out of here, which is a problem
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

    debug_print( L"to_long_double( ", DEBUG_ARITH );
    debug_print_object( arg, DEBUG_ARITH );
    debug_printf( DEBUG_ARITH, L") => %lf\n", result );

    return result;
}


/**
 * Return the closest possible `int64_t` representation to the value of
 * this `arg`, expected to be an integer, ratio or real, or `NAN` if `arg`
 * is not any of these.
 *
 * @arg a pointer to an integer, ratio or real.
 *
 * \todo cannot throw an exception out of here, which is a problem
 * if a ratio may legally have zero as a divisor, or something which is
 * not a number (or is a big number) is passed in.
 */
int64_t to_long_int( struct cons_pointer arg ) {
    int64_t result = 0;
    struct cons_space_object cell = pointer2cell( arg );
    switch ( cell.tag.value ) {
        case INTEGERTV:
            /* \todo if (integerp(cell.payload.integer.more)) {
             *     throw an exception!
             * } */
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
struct cons_pointer add_2( struct stack_frame *frame,
                           struct cons_pointer frame_pointer,
                           struct cons_pointer arg1,
                           struct cons_pointer arg2 ) {
    struct cons_pointer result;
    struct cons_space_object cell1 = pointer2cell( arg1 );
    struct cons_space_object cell2 = pointer2cell( arg2 );

    debug_print( L"add_2( arg1 = ", DEBUG_ARITH );
    debug_dump_object( arg1, DEBUG_ARITH );
    debug_print( L"; arg2 = ", DEBUG_ARITH );
    debug_dump_object( arg2, DEBUG_ARITH );
    debug_print( L"\n", DEBUG_ARITH );

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
                        result = add_integers( arg1, arg2 );
                        break;
                    case RATIOTV:
                        result =
                            add_integer_ratio( frame_pointer, arg1, arg2 );
                        break;
                    case REALTV:
                        result =
                            make_real( to_long_double( arg1 ) +
                                       to_long_double( arg2 ) );
                        break;
                    default:
                        result = throw_exception( c_string_to_lisp_string
                                                  ( L"Cannot add: not a number" ),
                                                  frame_pointer );
                        break;
                }
                break;
            case RATIOTV:
                switch ( cell2.tag.value ) {
                    case EXCEPTIONTV:
                        result = arg2;
                        break;
                    case INTEGERTV:
                        result =
                            add_integer_ratio( frame_pointer, arg2, arg1 );
                        break;
                    case RATIOTV:
                        result = add_ratio_ratio( frame_pointer, arg1, arg2 );
                        break;
                    case REALTV:
                        result =
                            make_real( to_long_double( arg1 ) +
                                       to_long_double( arg2 ) );
                        break;
                    default:
                        result = throw_exception( c_string_to_lisp_string
                                                  ( L"Cannot add: not a number" ),
                                                  frame_pointer );
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
                    throw_exception( c_string_to_lisp_string
                                     ( L"Cannot add: not a number" ),
                                     frame_pointer );
        }
    }

    debug_print( L"}; => ", DEBUG_ARITH );
    debug_print_object( arg2, DEBUG_ARITH );
    debug_print( L"\n", DEBUG_ARITH );

    return result;
}

/**
 * Add an indefinite number of numbers together
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer, ratio or real.
 * @exception if any argument is not a number, returns an exception.
 */
struct cons_pointer lisp_add( struct stack_frame
                              *frame, struct cons_pointer frame_pointer, struct
                              cons_pointer env ) {
    struct cons_pointer result = make_integer( 0, NIL );
    struct cons_pointer tmp;

    for ( int i = 0;
          i < args_in_frame &&
          !nilp( frame->arg[i] ) && !exceptionp( result ); i++ ) {
        tmp = result;
        result = add_2( frame, frame_pointer, result, frame->arg[i] );
        if ( !eq( tmp, result ) ) {
            dec_ref( tmp );
        }
    }

    struct cons_pointer more = frame->more;
    while ( consp( more ) && !exceptionp( result ) ) {
        tmp = result;
        result = add_2( frame, frame_pointer, result, c_car( more ) );
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
                                struct cons_pointer frame_pointer,
                                struct cons_pointer arg1,
                                struct cons_pointer arg2 ) {
    struct cons_pointer result;
    struct cons_space_object cell1 = pointer2cell( arg1 );
    struct cons_space_object cell2 = pointer2cell( arg2 );

    debug_print( L"multiply_2( arg1 = ", DEBUG_ARITH );
    debug_print_object( arg1, DEBUG_ARITH );
    debug_print( L"; arg2 = ", DEBUG_ARITH );
    debug_print_object( arg2, DEBUG_ARITH );
    debug_print( L")", DEBUG_ARITH );

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
                        result = multiply_integers( arg1, arg2 );
                        break;
                    case RATIOTV:
                        result =
                            multiply_integer_ratio( frame_pointer, arg1,
                                                    arg2 );
                        break;
                    case REALTV:
                        result =
                            make_real( to_long_double( arg1 ) *
                                       to_long_double( arg2 ) );
                        break;
                    default:
                        result = throw_exception( c_string_to_lisp_string
                                                  ( L"Cannot multiply: argument 2 is not a number" ),
                                                  frame_pointer );
                        break;
                }
                break;
            case RATIOTV:
                switch ( cell2.tag.value ) {
                    case EXCEPTIONTV:
                        result = arg2;
                        break;
                    case INTEGERTV:
                        result =
                            multiply_integer_ratio( frame_pointer, arg2,
                                                    arg1 );
                        break;
                    case RATIOTV:
                        result =
                            multiply_ratio_ratio( frame_pointer, arg1, arg2 );
                        break;
                    case REALTV:
                        result =
                            make_real( to_long_double( arg1 ) *
                                       to_long_double( arg2 ) );
                        break;
                    default:
                        result = throw_exception( c_string_to_lisp_string
                                                  ( L"Cannot multiply: argument 1 is not a number" ),
                                                  frame_pointer );
                }
                break;
            case REALTV:
                result = exceptionp( arg2 ) ? arg2 :
                    make_real( to_long_double( arg1 ) *
                               to_long_double( arg2 ) );
                break;
            default:
                result = throw_exception( c_string_to_lisp_string
                                          ( L"Cannot multiply: not a number" ),
                                          frame_pointer );
                break;
        }
    }

    debug_print( L" => ", DEBUG_ARITH );
    debug_print_object( arg2, DEBUG_ARITH );
    debug_print( L"\n", DEBUG_ARITH );

    return result;
}

/**
 * Multiply an indefinite number of numbers together
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer, ratio or real.
 * @exception if any argument is not a number, returns an exception.
 */
struct cons_pointer lisp_multiply( struct
                                   stack_frame
                                   *frame, struct cons_pointer frame_pointer, struct
                                   cons_pointer env ) {
    struct cons_pointer result = make_integer( 1, NIL );
    struct cons_pointer tmp;

    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] )
          && !exceptionp( result ); i++ ) {
        tmp = result;
        result = multiply_2( frame, frame_pointer, result, frame->arg[i] );

        if ( !eq( tmp, result ) ) {
            dec_ref( tmp );
        }
    }

    struct cons_pointer more = frame->more;
    while ( consp( more )
            && !exceptionp( result ) ) {
        tmp = result;
        result = multiply_2( frame, frame_pointer, result, c_car( more ) );

        if ( !eq( tmp, result ) ) {
            dec_ref( tmp );
        }

        more = c_cdr( more );
    }

    return result;
}

/**
 * return a cons_pointer indicating a number which is the
 * 0 - the number indicated by `arg`.
 */
struct cons_pointer negative( struct cons_pointer frame,
                              struct cons_pointer arg ) {
    struct cons_pointer result = NIL;
    struct cons_space_object cell = pointer2cell( arg );

    switch ( cell.tag.value ) {
        case EXCEPTIONTV:
            result = arg;
            break;
        case INTEGERTV:
            result =
                make_integer( 0 - cell.payload.integer.value,
                              cell.payload.integer.more );
            break;
        case NILTV:
            result = TRUE;
            break;
        case RATIOTV:
            result = make_ratio( frame,
                                 negative( frame,
                                           cell.payload.ratio.dividend ),
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
 * return a cons_pointer indicating a number which is the result of
 * subtracting the number indicated by `arg2` from that indicated by `arg1`,
 * in the context of this `frame`.
 */
struct cons_pointer subtract_2( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer arg1,
                                struct cons_pointer arg2 ) {
    struct cons_pointer result = NIL;

    switch ( pointer2cell( arg1 ).tag.value ) {
        case EXCEPTIONTV:
            result = arg1;
            break;
        case INTEGERTV:
            switch ( pointer2cell( arg2 ).tag.value ) {
                case EXCEPTIONTV:
                    result = arg2;
                    break;
                case INTEGERTV:{
                        struct cons_pointer i =
                            negative( frame_pointer, arg2 );
                        inc_ref( i );
                        result = add_integers( arg1, i );
                        dec_ref( i );
                    }
                    break;
                case RATIOTV:{
                        struct cons_pointer tmp =
                            make_ratio( frame_pointer, arg1,
                                        make_integer( 1, NIL ) );
                        inc_ref( tmp );
                        result =
                            subtract_ratio_ratio( frame_pointer, tmp, arg2 );
                        dec_ref( tmp );
                    }
                    break;
                case REALTV:
                    result =
                        make_real( to_long_double( arg1 ) -
                                   to_long_double( arg2 ) );
                    break;
                default:
                    result = throw_exception( c_string_to_lisp_string
                                              ( L"Cannot subtract: not a number" ),
                                              frame_pointer );
                    break;
            }
            break;
        case RATIOTV:
            switch ( pointer2cell( arg2 ).tag.value ) {
                case EXCEPTIONTV:
                    result = arg2;
                    break;
                case INTEGERTV:{
                        struct cons_pointer tmp =
                            make_ratio( frame_pointer, arg2,
                                        make_integer( 1, NIL ) );
                        inc_ref( tmp );
                        result =
                            subtract_ratio_ratio( frame_pointer, arg1, tmp );
                        dec_ref( tmp );
                    }
                    break;
                case RATIOTV:
                    result = subtract_ratio_ratio( frame_pointer, arg1, arg2 );
                    break;
                case REALTV:
                    result =
                        make_real( to_long_double( arg1 ) -
                                   to_long_double( arg2 ) );
                    break;
                default:
                    result = throw_exception( c_string_to_lisp_string
                                              ( L"Cannot subtract: not a number" ),
                                              frame_pointer );
                    break;
            }
            break;
        case REALTV:
            result = exceptionp( arg2 ) ? arg2 :
                make_real( to_long_double( arg1 ) - to_long_double( arg2 ) );
            break;
        default:
            result = throw_exception( c_string_to_lisp_string
                                      ( L"Cannot subtract: not a number" ),
                                      frame_pointer );
            break;
    }

    // and if not nilp[frame->arg[2]) we also have an error.

    return result;
}

/**
 * Subtract one number from another. If more than two arguments are passed
 * in the frame, the additional arguments are ignored.
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer, ratio or real.
 * @exception if either argument is not a number, returns an exception.
 */
struct cons_pointer lisp_subtract( struct
                                   stack_frame
                                   *frame, struct cons_pointer frame_pointer, struct
                                   cons_pointer env ) {
    return subtract_2( frame, frame_pointer, frame->arg[0], frame->arg[1] );
}

/**
 * Divide one number by another. If more than two arguments are passed
 * in the frame, the additional arguments are ignored.
 * @param env the evaluation environment - ignored;
 * @param frame the stack frame.
 * @return a pointer to an integer or real.
 * @exception if either argument is not a number, returns an exception.
 */
struct cons_pointer lisp_divide( struct
                                 stack_frame
                                 *frame, struct cons_pointer frame_pointer, struct
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
                            make_ratio( frame_pointer, frame->arg[0],
                                        frame->arg[1] );
                        /* OK, if result may be unsimplified, we should not inc_ref it
                         * - but if not, we should dec_ref it. */
                        result = simplify_ratio( frame_pointer, unsimplified );
                        if ( !eq( unsimplified, result ) ) {
                            dec_ref( unsimplified );
                        }
                    }
                    break;
                case RATIOTV:{
                        struct cons_pointer one = make_integer( 1, NIL );
                        struct cons_pointer ratio =
                            make_ratio( frame_pointer, frame->arg[0], one );
                        inc_ref( ratio );
                        result =
                            divide_ratio_ratio( frame_pointer, ratio,
                                                frame->arg[1] );
                        dec_ref( ratio );
                    }
                    break;
                case REALTV:
                    result =
                        make_real( to_long_double( frame->arg[0] ) /
                                   to_long_double( frame->arg[1] ) );
                    break;
                default:
                    result = throw_exception( c_string_to_lisp_string
                                              ( L"Cannot divide: not a number" ),
                                              frame_pointer );
                    break;
            }
            break;
        case RATIOTV:
            switch ( arg1.tag.value ) {
                case EXCEPTIONTV:
                    result = frame->arg[1];
                    break;
                case INTEGERTV:{
                        struct cons_pointer one = make_integer( 1, NIL );
                        inc_ref( one );
                        struct cons_pointer ratio =
                            make_ratio( frame_pointer, frame->arg[1], one );
                        inc_ref( ratio );
                        result =
                            divide_ratio_ratio( frame_pointer, frame->arg[0],
                                                ratio );
                        dec_ref( ratio );
                        dec_ref( one );
                    }
                    break;
                case RATIOTV:
                    result =
                        divide_ratio_ratio( frame_pointer, frame->arg[0],
                                            frame->arg[1] );
                    break;
                case REALTV:
                    result =
                        make_real( to_long_double( frame->arg[0] ) /
                                   to_long_double( frame->arg[1] ) );
                    break;
                default:
                    result = throw_exception( c_string_to_lisp_string
                                              ( L"Cannot divide: not a number" ),
                                              frame_pointer );
                    break;
            }
            break;
        case REALTV:
            result = exceptionp( frame->arg[1] ) ? frame->arg[1] :
                make_real( to_long_double( frame->arg[0] ) /
                           to_long_double( frame->arg[1] ) );
            break;
        default:
            result = throw_exception( c_string_to_lisp_string
                                      ( L"Cannot divide: not a number" ),
                                      frame_pointer );
            break;
    }

    return result;
}
