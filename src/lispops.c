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

    if ( consp( arg ) || stringp( arg ) || symbolp( arg ) ) {
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
    set_reg( next, 0, form );
    result = lisp_eval( next, env );

    if ( !exceptionp( result ) ) {
        /* if we're returning an exception, we should NOT free the
         * stack frame. Corollary is, when we free an exception, we
         * should free all the frames it's holding on to. */
        free_stack_frame( next );
    }

    return result;
}

/**
 * eval all the forms in this `list` in the context of this stack `frame`
 * and this `env`, and return a list of their values. If the arg passed as
 * `list` is not in fact a list, return nil.
 */
struct cons_pointer eval_forms( struct stack_frame *frame,
                                struct cons_pointer list,
                                struct cons_pointer env ) {
    return consp( list ) ?
        make_cons( eval_form( frame, c_car( list ), env ),
                   eval_forms( frame, c_cdr( list ), env ) ) : NIL;
}

/**
 * Return the object list (root namespace).
 *
 * (oblist)
 */
struct cons_pointer
lisp_oblist( struct stack_frame *frame, struct cons_pointer env ) {
    return oblist;
}


/**
 * used to construct the body for `lambda` and `nlambda` expressions.
 */
struct cons_pointer compose_body( struct stack_frame *frame ) {
    struct cons_pointer body =
        !nilp( frame->arg[args_in_frame - 1] ) ? frame->more : NIL;

    for ( int i = args_in_frame - 1; i > 0; i-- ) {
        if ( !nilp( body ) ) {
            body = make_cons( frame->arg[i], body );
        } else if ( !nilp( frame->arg[i] ) ) {
            body = make_cons( frame->arg[i], body );
        }
    }

    fputws( L"compose_body returning ", stderr );
    print( stderr, body );
    fputws( L"\n", stderr );

    return body;
}

/**
 * Construct an interpretable function.
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param env the environment in which it is to be intepreted.
 */
struct cons_pointer
lisp_lambda( struct stack_frame *frame, struct cons_pointer env ) {
    return make_lambda( frame->arg[0], compose_body( frame ) );
}

/**
 * Construct an interpretable special form.
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param env the environment in which it is to be intepreted.
 */
struct cons_pointer
lisp_nlambda( struct stack_frame *frame, struct cons_pointer env ) {
    return make_nlambda( frame->arg[0], compose_body( frame ) );
}

void log_binding( struct cons_pointer name, struct cons_pointer val ) {
    print( stderr, c_string_to_lisp_string( "\n\tBinding " ) );
    print( stderr, name );
    print( stderr, c_string_to_lisp_string( " to " ) );
    print( stderr, val );
    fputws( L"\"\n", stderr );
}

/**
 * Evaluate a lambda or nlambda expression.
 */
struct cons_pointer
eval_lambda( struct cons_space_object cell, struct stack_frame *frame,
             struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    fwprintf( stderr, L"eval_lambda called\n" );

    struct cons_pointer new_env = env;
    struct cons_pointer names = cell.payload.lambda.args;
    struct cons_pointer body = cell.payload.lambda.body;

    if ( consp( names ) ) {
        /* if `names` is a list, bind successive items from that list
         * to values of arguments */
        for ( int i = 0; i < args_in_frame && consp( names ); i++ ) {
            struct cons_pointer name = c_car( names );
            struct cons_pointer val = frame->arg[i];

            new_env = bind( name, val, new_env );
            log_binding( name, val );

            names = c_cdr( names );
        }
    } else if ( symbolp( names ) ) {
        /* if `names` is a symbol, rather than a list of symbols,
         * then bind a list of the values of args to that symbol. */
        struct cons_pointer vals = frame->more;

        for ( int i = args_in_frame - 1; i >= 0; i-- ) {
            struct cons_pointer val = eval_form( frame, frame->arg[i], env );

            if ( nilp( val ) && nilp( vals ) ) {  /* nothing */
            } else {
                vals = make_cons( val, vals );
            }
        }

        new_env = bind( names, vals, new_env );
    }

    while ( !nilp( body ) ) {
        struct cons_pointer sexpr = c_car( body );
        body = c_cdr( body );
        fputws( L"In lambda: ", stderr );
        result = eval_form( frame, sexpr, new_env );
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
    set_reg( fn_frame, 0, c_car( frame->arg[0] ) );
    struct cons_pointer fn_pointer = lisp_eval( fn_frame, env );

    if ( !exceptionp( result ) ) {
        /* if we're returning an exception, we should NOT free the
         * stack frame. Corollary is, when we free an exception, we
         * should free all the frames it's holding on to. */
        free_stack_frame( fn_frame );
    }

    struct cons_space_object fn_cell = pointer2cell( fn_pointer );
    struct cons_pointer args = c_cdr( frame->arg[0] );

    switch ( fn_cell.tag.value ) {
        case EXCEPTIONTV:
            /* just pass exceptions straight back */
            result = fn_pointer;
            break;
        case FUNCTIONTV:
            {
                struct cons_pointer exep = NIL;
                struct stack_frame *next =
                    make_stack_frame( frame, args, env, &exep );
                result = ( *fn_cell.payload.special.executable ) ( next, env );
                if ( exceptionp( exep ) ) {
                    /* if we're returning an exception, we should NOT free the
                     * stack frame. Corollary is, when we free an exception, we
                     * should free all the frames it's holding on to. */
                    result = exep;
                } else {
                    free_stack_frame( next );
                }
            }
            break;
        case LAMBDATV:
            {
                struct cons_pointer exep = NIL;
                struct stack_frame *next =
                    make_stack_frame( frame, args, env, &exep );
                fputws( L"Stack frame for lambda\n", stderr );
                dump_frame( stderr, next );
                result = eval_lambda( fn_cell, next, env );
                if ( exceptionp( result ) ) {
                    /* if we're returning an exception, we should NOT free the
                     * stack frame. Corollary is, when we free an exception, we
                     * should free all the frames it's holding on to. */
                    result = exep;
                } else {
                    free_stack_frame( next );
                }
            }
            break;
        case NLAMBDATV:
            {
                struct stack_frame *next =
                    make_special_frame( frame, args, env );
                fputws( L"Stack frame for nlambda\n", stderr );
                dump_frame( stderr, next );
                result = eval_lambda( fn_cell, next, env );
                if ( !exceptionp( result ) ) {
                    /* if we're returning an exception, we should NOT free the
                     * stack frame. Corollary is, when we free an exception, we
                     * should free all the frames it's holding on to. */
                    free_stack_frame( next );
                }
            }
            break;
        case SPECIALTV:
            {
                struct stack_frame *next =
                    make_special_frame( frame, args, env );
                result = ( *fn_cell.payload.special.executable ) ( next, env );
                if ( !exceptionp( result ) ) {
                    /* if we're returning an exception, we should NOT free the
                     * stack frame. Corollary is, when we free an exception, we
                     * should free all the frames it's holding on to. */
                    free_stack_frame( next );
                }
            }
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
            {
                result = c_apply( frame, env );
            }
            break;

        case SYMBOLTV:
            {
                struct cons_pointer canonical =
                    internedp( frame->arg[0], env );
                if ( nilp( canonical ) ) {
                    struct cons_pointer message =
                        make_cons( c_string_to_lisp_string
                                   ( "Attempt to take value of unbound symbol." ),
                                   frame->arg[0] );
                    result = lisp_throw( message, frame );
                } else {
                    result = c_assoc( canonical, env );
                    inc_ref( result );
                }
            }
            break;
            /*
             * TODO:
             * the Clojure practice of having a map serve in the function place of
             * an s-expression is a good one and I should adopt it; also if the
             * object is a consp it could be interpretable source code but in the
             * long run I don't want an interpreter, and if I can get away without
             * so much the better.
             */
        default:
            result = frame->arg[0];
            break;
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

    set_reg( frame, 0, make_cons( frame->arg[0], frame->arg[1] ) );
    set_reg( frame, 1, NIL );

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
 * (set name value)
 * (set name value namespace)
 *
 * Function.
 * `namespace` defaults to the oblist.
 * Binds the value of `name` in the `namespace` to value of `value`, altering
 * the namespace in so doing. `namespace` defaults to the value of `oblist`.
 */
struct cons_pointer
lisp_set( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_pointer namespace =
        nilp( frame->arg[2] ) ? oblist : frame->arg[2];

    if ( symbolp( frame->arg[0] ) ) {
        deep_bind( frame->arg[0], frame->arg[1] );
        result = frame->arg[1];
    } else {
        result =
            make_exception( make_cons
                            ( c_string_to_lisp_string
                              ( "The first argument to `set!` is not a symbol: " ),
                              make_cons( frame->arg[0], NIL ) ), frame );
    }

    return result;
}


/**
 * (set! symbol value)
 * (set! symbol value namespace)
 *
 * Special form.
 * `namespace` defaults to the oblist.
 * Binds `symbol` in the `namespace` to value of `value`, altering
 * the namespace in so doing. `namespace` defaults to the value of `oblist`.
 */
struct cons_pointer
lisp_set_shriek( struct stack_frame *frame, struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_pointer namespace =
        nilp( frame->arg[2] ) ? oblist : frame->arg[2];

    if ( symbolp( frame->arg[0] ) ) {
        struct cons_pointer val = eval_form( frame, frame->arg[1], env );
        deep_bind( frame->arg[0], val );
        result = val;
    } else {
        result =
            make_exception( make_cons
                            ( c_string_to_lisp_string
                              ( "The first argument to `set!` is not a symbol: " ),
                              make_cons( frame->arg[0], NIL ) ), frame );
    }

    return result;
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
    struct cons_pointer result = NIL;
    FILE *output = stdout;

    if ( writep( frame->arg[1] ) ) {
        output = pointer2cell( frame->arg[1] ).payload.stream.stream;
    }

    result = print( output, frame->arg[0] );

  fputws( L"Print returning ", stderr);
  print(stderr, result);
  fputws( L"\n", stderr);

    return result;
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
 * (progn forms...)
 *
 * Special form; evaluate the forms which are listed in my arguments
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
    struct cons_pointer remaining = frame->more;
    struct cons_pointer result = NIL;

    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] ); i++ ) {
        result = eval_form( frame, frame->arg[i], env );
    }

    while ( consp( remaining ) ) {
        result = eval_form( frame, c_car( remaining ), env );

        remaining = c_cdr( remaining );
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
            result = eval_form( frame, c_car( clause_pointer ), env );

            if ( !nilp( result ) ) {
                struct cons_pointer vals =
                    eval_forms( frame, c_cdr( clause_pointer ), env );

                while ( consp( vals ) ) {
                    result = c_car( vals );
                    vals = c_cdr( vals );
                }

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
 * This requires that a frame be a heap-space object with a cons-space
 * object pointing to it. Then this should become a normal lisp function
 * which expects a normally bound frame and environment, such that
 * frame->arg[0] is the message, and frame->arg[1] is the cons-space
 * pointer to the frame in which the exception occurred.
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
