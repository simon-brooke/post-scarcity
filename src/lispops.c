/*
 * lispops.c
 *
 * List processing operations.
 *
 * The general idea here is that a list processing operation is a
 * function which takes two arguments, both cons_pointers:
 *
 * 1. args, the argument list to this function;
 * 2. env, the environment in which this function should be evaluated;
 *
 * and returns a cons_pointer, the result.
 *
 * They must all have the same signature so that I can call them as
 * function pointers.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "equal.h"
#include "integer.h"
#include "intern.h"
#include "lispops.h"
#include "print.h"
#include "read.h"
#include "stack.h"

/*
 * also to create in this section:
 * struct cons_pointer lisp_let( struct cons_pointer args, struct cons_pointer env,
 struct stack_frame* frame);
 * struct cons_pointer lisp_mapcar( struct cons_pointer args, struct cons_pointer env,
 struct stack_frame* frame);
 *
 * and others I haven't thought of yet.
 */

/**
 * Implementation of car in C. If arg is not a cons, does not error but returns nil.
 */
struct cons_pointer c_car( struct cons_pointer arg ) {
    struct cons_pointer result = NIL;

    if ( consp( arg ) ) {
        result = pointer2cell( arg ).payload.cons.car;
    }

    return result;
}

/**
 * Implementation of cdr in C. If arg is not a cons, does not error but returns nil.
 */
struct cons_pointer c_cdr( struct cons_pointer arg ) {
    struct cons_pointer result = NIL;

    if ( consp( arg ) ) {
        result = pointer2cell( arg ).payload.cons.cdr;
    }

    return result;
}


/**
 * Useful building block; evaluate this single form in the context of this
 * parent stack frame and this environment.
 * @param parent the parent stack frame.
 * @param form the form to be evaluated.
 * @param env the evaluation environment.
 * @return the result of evaluating the form.
 */
struct cons_pointer eval_form( struct stack_frame *parent,
                               struct cons_pointer form,
                               struct cons_pointer env ) {
    fputws( L"eval_form: ", stderr );
    print( stderr, form );
    fputws( L"\n", stderr );

    struct cons_pointer result = NIL;
    struct stack_frame *next = make_empty_frame( parent, env );
    next->arg[0] = form;
    inc_ref( next->arg[0] );
    result = lisp_eval( next, env );
    free_stack_frame( next );

    return result;
}

/**
 * The Lisp interpreter.
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param lexpr the lambda expression to be interpreted;
 * @param env the environment in which it is to be intepreted.
 */
struct cons_pointer
lisp_lambda( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_pointer lexpr = frame->arg[0];
    struct cons_pointer should_be_lambda =
        eval_form( frame, c_car( lexpr ), env );

    dump_frame( stderr, frame );

    if ( lambdap( should_be_lambda ) ) {
        struct cons_pointer new_env = env;
        struct cons_pointer args = c_car( c_cdr( lexpr ) );
        struct cons_pointer body = c_cdr( c_cdr( lexpr ) );

        for ( int i = 1; i < args_in_frame && consp( args ); i++ ) {
            args = c_cdr( args );
            struct cons_pointer arg = c_car( args );
            print( stderr, c_string_to_lisp_string( "\n\tBinding " ) );
            print( stderr, arg );
            print( stderr, c_string_to_lisp_string( " to " ) );
            print( stderr, frame->arg[i] );

            new_env = make_cons( make_cons( arg, frame->arg[i] ), new_env );
        }

        while ( !nilp( body ) ) {
            struct cons_pointer sexpr = c_car( body );
            body = c_cdr( body );

            result = eval_form( frame, sexpr, new_env );
        }
    } else {
        if ( exceptionp( should_be_lambda ) ) {
            result = should_be_lambda;
        } else {
            char *buffer = malloc( 1024 );
            struct cons_space_object not_lambda =
                pointer2cell( should_be_lambda );
            memset( buffer, '\0', 1024 );
            sprintf( buffer,
                     "Expected lambda, but found cell with tag %d (%c%c%c%c)",
                     not_lambda.tag.value, not_lambda.tag.bytes[0],
                     not_lambda.tag.bytes[1], not_lambda.tag.bytes[2],
                     not_lambda.tag.bytes[3] );
            struct cons_pointer message = c_string_to_lisp_string( buffer );
            free( buffer );
            result = lisp_throw( message, frame );
        }
    }
    return result;
}



/**
 * Internal guts of apply.
 * @param frame the stack frame, expected to have only one argument, a list
 * comprising something that evaluates to a function and its arguments.
 * @param env The evaluation environment.
 * @return the result of evaluating the function with its arguments.
 */
struct cons_pointer
c_apply( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    struct stack_frame *fn_frame = make_empty_frame( frame, env );
    fn_frame->arg[0] = c_car( frame->arg[0] );
    inc_ref( fn_frame->arg[0] );
    struct cons_pointer fn_pointer = lisp_eval( fn_frame, env );
    free_stack_frame( fn_frame );

    struct cons_space_object fn_cell = pointer2cell( fn_pointer );
    struct cons_pointer args = c_cdr( frame->arg[0] );

    switch ( fn_cell.tag.value ) {
        case SPECIALTV:
            {
                struct stack_frame *next =
                    make_special_frame( frame, args, env );
                result = ( *fn_cell.payload.special.executable ) ( next, env );
                free_stack_frame( next );
            }
            break;
        case FUNCTIONTV:
            {
                struct stack_frame *next =
                    make_stack_frame( frame, args, env );
                result = ( *fn_cell.payload.special.executable ) ( next, env );
                free_stack_frame( next );
            }
            break;
        case CONSTV:
            {
                fwprintf( stdout,
                          L"Treating cons as lambda expression (apply)\n" );
                result = lisp_lambda( frame, env );
            }
            break;
        case EXCEPTIONTV:
            /* just pass exceptions straight back */
            result = fn_pointer;
            break;
        default:
            {
                char *buffer = malloc( 1024 );
                memset( buffer, '\0', 1024 );
                sprintf( buffer,
                         "Unexpected cell with tag %d (%c%c%c%c) in function position",
                         fn_cell.tag.value, fn_cell.tag.bytes[0],
                         fn_cell.tag.bytes[1], fn_cell.tag.bytes[2],
                         fn_cell.tag.bytes[3] );
                struct cons_pointer message =
                    c_string_to_lisp_string( buffer );
                free( buffer );
                result = lisp_throw( message, frame );
            }
    }

    return result;
}


/**
 * Get the Lisp type of the single argument.
 * @param pointer a pointer to the object whose type is requested.
 * @return As a Lisp string, the tag of the object which is at that pointer.
 */
struct cons_pointer c_type( struct cons_pointer pointer ) {
    char *buffer = malloc( TAGLENGTH + 1 );
    memset( buffer, 0, TAGLENGTH + 1 );
    struct cons_space_object cell = pointer2cell( pointer );
    strncpy( buffer, cell.tag.bytes, TAGLENGTH );

    struct cons_pointer result = c_string_to_lisp_string( buffer );
    free( buffer );

    return result;
}


/**
 * (eval s_expr)
 *
 * function.
 * If s_expr is a number, NIL, or T, returns s_expr.
 * If s_expr is an unprotected string, returns the value that s_expr is bound
 * to in the evaluation environment (env).
 * If s_expr is a list, expects the car to be something that evaluates to a
 * function or special form.
 * If a function, evaluates all the other top level elements in s_expr and
 * passes them in a stack frame as arguments to the function.
 * If a special form, passes the cdr of s_expr to the special form as argument.
 */
struct cons_pointer
lisp_eval( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = frame->arg[0];
    struct cons_space_object cell = pointer2cell( frame->arg[0] );

    fputws( L"Eval: ", stderr );
    dump_frame( stderr, frame );

    switch ( cell.tag.value ) {
        case CONSTV:
            print( stderr, frame->arg[0] );
            if ( lambdap( c_car( frame->arg[0] ) ) ) {
                fwprintf( stdout,
                          L"Treating cons as lambda expression (eval)\n" );
                result = lisp_lambda( frame, env );
            } else {
                result = c_apply( frame, env );
            }
            break;

        case SYMBOLTV:
            {
                struct cons_pointer canonical =
                    internedp( frame->arg[0], env );
                if ( nilp( canonical ) ) {
                    struct cons_pointer message =
                        c_string_to_lisp_string
                        ( "Attempt to take value of unbound symbol." );
                    result = lisp_throw( message, frame );
                } else {
                    result = c_assoc( canonical, env );
                }
            }
            break;
            /*
             * the Clojure practice of having a map serve in the function place of
             * an s-expression is a good one and I should adopt it; also if the
             * object is a consp it could be interpretable source code but in the
             * long run I don't want an interpreter, and if I can get away without
             * so much the better.
             */
    }

    fputws( L"Eval returning ", stderr );
    print( stderr, result );
    fputws( L"\n", stderr );

    return result;
}


/**
 * (apply fn args)
 *
 * function. Apply the function which is the result of evaluating the
 * first argoment to the list of arguments which is the result of evaluating
 * the second argument
 */
struct cons_pointer
lisp_apply( struct stack_frame *frame, struct cons_pointer env ) {
    fputws( L"Apply: ", stderr );
    dump_frame( stderr, frame );

    frame->arg[0] = make_cons( frame->arg[0], frame->arg[1] );
    inc_ref( frame->arg[0] );
    frame->arg[1] = NIL;

    struct cons_pointer result = c_apply( frame, env );

    fputws( L"Apply returning ", stderr );
    print( stderr, result );
    fputws( L"\n", stderr );

    return result;
}


/**
 * (quote a)
 *
 * Special form
 * Returns its argument (strictly first argument - only one is expected but
 * this isn't at this stage checked) unevaluated.
 */
struct cons_pointer
lisp_quote( struct stack_frame *frame, struct cons_pointer env ) {
    return frame->arg[0];
}

/**
 * (cons a b)
 *
 * Function.
 * Returns a cell constructed from a and b. If a is of type string but its
 * cdr is nill, and b is of type string, then returns a new string cell;
 * otherwise returns a new cons cell.
 */
struct cons_pointer
lisp_cons( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer car = frame->arg[0];
    struct cons_pointer cdr = frame->arg[1];
    struct cons_pointer result;

    if ( nilp( car ) && nilp( cdr ) ) {
        return NIL;
    } else if ( stringp( car ) && stringp( cdr ) &&
                nilp( pointer2cell( car ).payload.string.cdr ) ) {
        result =
            make_string( pointer2cell( car ).payload.string.character, cdr );
    } else {
        result = make_cons( car, cdr );
    }

    return result;
}

/**
 * (car s_expr)
 * Returns the first item (head) of a sequence. Valid for cons cells,
 * strings, and TODO read streams and other things which can be considered as sequences.
 */
struct cons_pointer
lisp_car( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    if ( consp( frame->arg[0] ) ) {
        struct cons_space_object cell = pointer2cell( frame->arg[0] );
        result = cell.payload.cons.car;
    } else if ( stringp( frame->arg[0] ) ) {
        struct cons_space_object cell = pointer2cell( frame->arg[0] );
        result = make_string( cell.payload.string.character, NIL );
    } else {
        struct cons_pointer message =
            c_string_to_lisp_string( "Attempt to take CAR of non sequence" );
        result = lisp_throw( message, frame );
    }

    return result;
}

/**
 * (cdr s_expr)
 * Returns the remainder of a sequence when the head is removed. Valid for cons cells,
 * strings, and TODO read streams and other things which can be considered as sequences.
 */
struct cons_pointer
lisp_cdr( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    if ( consp( frame->arg[0] ) ) {
        struct cons_space_object cell = pointer2cell( frame->arg[0] );
        result = cell.payload.cons.cdr;
    } else if ( stringp( frame->arg[0] ) ) {
        struct cons_space_object cell = pointer2cell( frame->arg[0] );
        result = cell.payload.string.cdr;
    } else {
        struct cons_pointer message =
            c_string_to_lisp_string( "Attempt to take CDR of non sequence" );
        result = lisp_throw( message, frame );
    }

    return result;
}

/**
 * (assoc key store)
 * Returns the value associated with key in store, or NIL if not found.
 */
struct cons_pointer
lisp_assoc( struct stack_frame *frame, struct cons_pointer env ) {
    return c_assoc( frame->arg[0], frame->arg[1] );
}

/**
 * (eq a b)
 * Returns T if a and b are pointers to the same object, else NIL
 */
struct cons_pointer lisp_eq( struct stack_frame *frame,
                             struct cons_pointer env ) {
    return eq( frame->arg[0], frame->arg[1] ) ? TRUE : NIL;
}

/**
 * (eq a b)
 * Returns T if a and b are pointers to structurally identical objects, else NIL
 */
struct cons_pointer
lisp_equal( struct stack_frame *frame, struct cons_pointer env ) {
    return equal( frame->arg[0], frame->arg[1] ) ? TRUE : NIL;
}

/**
 * (read)
 * (read read-stream)
 * Read one complete lisp form and return it. If read-stream is specified and
 * is a read stream, then read from that stream, else stdin.
 */
struct cons_pointer
lisp_read( struct stack_frame *frame, struct cons_pointer env ) {
    FILE *input = stdin;

    if ( readp( frame->arg[0] ) ) {
        input = pointer2cell( frame->arg[0] ).payload.stream.stream;
    }

    return read( frame, input );
}


/**
 * (print expr)
 * (print expr write-stream)
 * Print one complete lisp form and return NIL. If write-stream is specified and
 * is a write stream, then print to that stream, else stdout.
 */
struct cons_pointer
lisp_print( struct stack_frame *frame, struct cons_pointer env ) {
    FILE *output = stdout;

    if ( writep( frame->arg[1] ) ) {
        output = pointer2cell( frame->arg[1] ).payload.stream.stream;
    }

    print( output, frame->arg[0] );

    return NIL;
}


/**
 * Function: Get the Lisp type of the single argument.
 * @param frame My stack frame.
 * @param env My environment (ignored).
 * @return As a Lisp string, the tag of the object which is the argument.
 */
struct cons_pointer
lisp_type( struct stack_frame *frame, struct cons_pointer env ) {
    return c_type( frame->arg[0] );
}


/**
 * Function; evaluate the forms which are listed in my single argument
 * sequentially and return the value of the last. This function is called 'do'
 * in some dialects of Lisp.
 *
 * @param frame My stack frame.
 * @param env My environment (ignored).
 * @return the value of the last form on the sequence which is my single
 * argument.
 */
struct cons_pointer
lisp_progn( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer remaining = frame->arg[0];
    struct cons_pointer result = NIL;

    while ( consp( remaining ) ) {
        struct cons_space_object cell = pointer2cell( remaining );
        result = eval_form( frame, cell.payload.cons.car, env );

        remaining = cell.payload.cons.cdr;
    }

    return result;
}

/**
 * Special form: conditional. Each arg is expected to be a list; if the first
 * item in such a list evaluates to non-NIL, the remaining items in that list
 * are evaluated in turn and the value of the last returned. If no arg (clause)
 * has a first element which evaluates to non NIL, then NIL is returned.
 * @param frame My stack frame.
 * @param env My environment (ignored).
 * @return the value of the last form of the first successful clause.
 */
struct cons_pointer
lisp_cond( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    bool done = false;

    for ( int i = 0; i < args_in_frame && !done; i++ ) {
        struct cons_pointer clause_pointer = frame->arg[i];
        fputws( L"Cond clause: ", stderr );
        print( stderr, clause_pointer );

        if ( consp( clause_pointer ) ) {
            struct cons_space_object cell = pointer2cell( clause_pointer );

            if ( !nilp( eval_form( frame, cell.payload.cons.car, env ) ) ) {
                struct stack_frame *next = make_empty_frame( frame, env );
                next->arg[0] = cell.payload.cons.cdr;
                inc_ref( next->arg[0] );
                result = lisp_progn( next, env );
                done = true;
            }
        } else if ( nilp( clause_pointer ) ) {
            done = true;
        } else {
            result = lisp_throw( c_string_to_lisp_string
                                 ( "Arguments to `cond` must be lists" ),
                                 frame );
        }
    }
    /* TODO: if there are more than 8 clauses we need to continue into the
     * remainder */

    return result;
}

/**
 * TODO: make this do something sensible somehow.
 */
struct cons_pointer
lisp_throw( struct cons_pointer message, struct stack_frame *frame ) {
    fwprintf( stderr, L"\nERROR: " );
    print( stderr, message );
    struct cons_pointer result = NIL;

    struct cons_space_object cell = pointer2cell( message );

    if ( cell.tag.value == EXCEPTIONTV ) {
        result = message;
    } else {
        result = make_exception( message, frame );
    }

    return result;
}
