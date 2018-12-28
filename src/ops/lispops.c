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
#include "debug.h"
#include "dump.h"
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
                               struct cons_pointer parent_pointer,
                               struct cons_pointer form,
                               struct cons_pointer env ) {
    debug_print( L"eval_form: ", DEBUG_EVAL );
    debug_dump_object( form, DEBUG_EVAL );

    struct cons_pointer result = NIL;
    struct cons_pointer next_pointer = make_empty_frame( parent_pointer );
    inc_ref( next_pointer );

    struct stack_frame *next = get_stack_frame( next_pointer );
    set_reg( next, 0, form );
    next->args = 1;

    result = lisp_eval( next, next_pointer, env );

    if ( !exceptionp( result ) ) {
        /* if we're returning an exception, we should NOT free the
         * stack frame. Corollary is, when we free an exception, we
         * should free all the frames it's holding on to. */
        dec_ref( next_pointer );
    }

    return result;
}

/**
 * eval all the forms in this `list` in the context of this stack `frame`
 * and this `env`, and return a list of their values. If the arg passed as
 * `list` is not in fact a list, return nil.
 */
struct cons_pointer eval_forms( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer list,
                                struct cons_pointer env ) {
    /* TODO: refactor. This runs up the C stack. */
    return consp( list ) ?
        make_cons( eval_form( frame, frame_pointer, c_car( list ), env ),
                   eval_forms( frame, frame_pointer, c_cdr( list ),
                               env ) ) : NIL;
}

/**
 * Return the object list (root namespace).
 *
 * (oblist)
 */
struct cons_pointer
lisp_oblist( struct stack_frame *frame, struct cons_pointer frame_pointer,
             struct cons_pointer env ) {
    return oblist;
}


/**
 * used to construct the body for `lambda` and `nlambda` expressions.
 */
struct cons_pointer compose_body( struct stack_frame *frame ) {
    struct cons_pointer body = frame->more;

    for ( int i = args_in_frame - 1; i > 0; i-- ) {
        if ( !nilp( body ) ) {
            body = make_cons( frame->arg[i], body );
        } else if ( !nilp( frame->arg[i] ) ) {
            body = make_cons( frame->arg[i], body );
        }
    }

    debug_print( L"compose_body returning ", DEBUG_LAMBDA );
    debug_dump_object( body, DEBUG_LAMBDA );

    return body;
}

/**
 * Construct an interpretable function.
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param env the environment in which it is to be intepreted.
 */
struct cons_pointer
lisp_lambda( struct stack_frame *frame, struct cons_pointer frame_pointer,
             struct cons_pointer env ) {
    return make_lambda( frame->arg[0], compose_body( frame ) );
}

/**
 * Construct an interpretable special form.
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param env the environment in which it is to be intepreted.
 */
struct cons_pointer
lisp_nlambda( struct stack_frame *frame, struct cons_pointer frame_pointer,
              struct cons_pointer env ) {
    return make_nlambda( frame->arg[0], compose_body( frame ) );
}

void log_binding( struct cons_pointer name, struct cons_pointer val ) {
    debug_print( L"\n\tBinding ", DEBUG_ALLOC );
    debug_dump_object( name, DEBUG_ALLOC );
    debug_print( L" to ", DEBUG_ALLOC );
    debug_dump_object( val, DEBUG_ALLOC );
}

/**
 * Evaluate a lambda or nlambda expression.
 */
struct cons_pointer
eval_lambda( struct cons_space_object cell, struct stack_frame *frame,
             struct cons_pointer frame_pointer, struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    debug_print( L"eval_lambda called\n", DEBUG_EVAL );

    struct cons_pointer new_env = env;
    struct cons_pointer names = cell.payload.lambda.args;
    struct cons_pointer body = cell.payload.lambda.body;

    if ( consp( names ) ) {
        /* if `names` is a list, bind successive items from that list
         * to values of arguments */
        for ( int i = 0; i < frame->args && consp( names ); i++ ) {
            struct cons_pointer name = c_car( names );
            struct cons_pointer val = frame->arg[i];

            new_env = bind( name, val, new_env );
            log_binding( name, val );

            names = c_cdr( names );
        }
      /* TODO: if there's more than `args_in_frame` arguments, bind those too. */
    } else if ( symbolp( names ) ) {
        /* if `names` is a symbol, rather than a list of symbols,
         * then bind a list of the values of args to that symbol. */
        /* TODO: eval all the things in frame->more */
        struct cons_pointer vals = frame->more;

        for ( int i = args_in_frame - 1; i >= 0; i-- ) {
            struct cons_pointer val =
                eval_form( frame, frame_pointer, frame->arg[i], env );

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

        debug_print( L"In lambda: ", DEBUG_LAMBDA );

        result = eval_form( frame, frame_pointer, sexpr, new_env );
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
c_apply( struct stack_frame *frame, struct cons_pointer frame_pointer,
         struct cons_pointer env ) {
  debug_print(L"Entering c_apply\n", DEBUG_EVAL);
  struct cons_pointer result = NIL;

    struct cons_pointer fn_pointer =
        eval_form( frame, frame_pointer, c_car( frame->arg[0] ), env );

    if ( exceptionp( fn_pointer ) ) {
        result = fn_pointer;
    } else {
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
                struct cons_pointer next_pointer =
                    make_stack_frame( frame_pointer, args, env );
                inc_ref( next_pointer );
                if ( exceptionp( next_pointer ) ) {
                    result = next_pointer;
                } else {
                    struct stack_frame *next = get_stack_frame( next_pointer );

                    result =
                        ( *fn_cell.payload.function.executable ) ( next,
                                                                   next_pointer,
                                                                   env );
                    dec_ref( next_pointer );
                }
            }
            break;
        case LAMBDATV:
            {
                struct cons_pointer exep = NIL;
                struct cons_pointer next_pointer =
                    make_stack_frame( frame_pointer, args, env );
                inc_ref( next_pointer );
                if ( exceptionp( next_pointer ) ) {
                    result = next_pointer;
                } else {
                    struct stack_frame *next = get_stack_frame( next_pointer );
                    result = eval_lambda( fn_cell, next, next_pointer, env );
                    if ( !exceptionp( result ) ) {
                        dec_ref( next_pointer );
                    }
                }
            }
            break;
        case NLAMBDATV:
            {
                struct cons_pointer next_pointer =
                    make_special_frame( frame_pointer, args, env );
                inc_ref( next_pointer );
                if ( exceptionp( next_pointer ) ) {
                    result = next_pointer;
                } else {
                    struct stack_frame *next =
                        get_stack_frame( next_pointer );
                    result = eval_lambda( fn_cell, next, next_pointer, env );
                        dec_ref( next_pointer );
                }
            }
            break;
        case SPECIALTV:
            {
                struct cons_pointer next_pointer =
                    make_special_frame( frame_pointer, args, env );
                inc_ref( next_pointer );
                if ( exceptionp( next_pointer ) ) {
                    result = next_pointer;
                } else {
                    result =
                        ( *fn_cell.payload.special.executable ) ( get_stack_frame( next_pointer ),
                                                                  next_pointer,
                                                                  env );
                   debug_print(L"Special form returning: ", DEBUG_EVAL);
                  debug_print_object(result, DEBUG_EVAL);
                  debug_println(DEBUG_EVAL);
                    dec_ref( next_pointer );
                }
            }
            break;
        default:
            {
              int bs = sizeof(wchar_t) * 1024;
                wchar_t *buffer = malloc( bs );
                memset( buffer, '\0', bs );
                swprintf( buffer, bs,
                         L"Unexpected cell with tag %d (%4.4s) in function position",
                         fn_cell.tag.value, &fn_cell.tag.bytes[0] );
                struct cons_pointer message =
                    c_string_to_lisp_string( buffer );
                free( buffer );
                result = throw_exception( message, frame_pointer );
            }
    }
    }

    debug_print(L"c_apply: returning: ", DEBUG_EVAL);
    debug_print_object(result, DEBUG_EVAL);
    debug_println(DEBUG_EVAL);

    return result;
}


/**
 * Get the Lisp type of the single argument.
 * @param pointer a pointer to the object whose type is requested.
 * @return As a Lisp string, the tag of the object which is at that pointer.
 */
struct cons_pointer c_type( struct cons_pointer pointer ) {
    struct cons_pointer result = NIL;
    struct cons_space_object cell = pointer2cell( pointer );

    for (int i = TAGLENGTH -1; i >= 0; i--)
    {
      result = make_string((wchar_t)cell.tag.bytes[i], result);
    }

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
lisp_eval( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
    debug_print( L"Eval: ", DEBUG_EVAL );
    debug_dump_object( frame_pointer, DEBUG_EVAL );

    struct cons_pointer result = frame->arg[0];
    struct cons_space_object cell = pointer2cell( frame->arg[0] );

    switch ( cell.tag.value ) {
        case CONSTV:
            {
                result = c_apply( frame, frame_pointer, env );
            }
            break;

        case SYMBOLTV:
            {
                struct cons_pointer canonical =
                    internedp( frame->arg[0], env );
                if ( nilp( canonical ) ) {
                    struct cons_pointer message =
                        make_cons( c_string_to_lisp_string
                                   ( L"Attempt to take value of unbound symbol." ),
                                   frame->arg[0] );
                    result = throw_exception( message, frame_pointer );
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

    debug_print( L"Eval returning ", DEBUG_EVAL );
    debug_dump_object( result, DEBUG_EVAL );

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
lisp_apply( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
#ifdef DEBUG
    debug_print( L"Apply: ", DEBUG_EVAL );
    dump_frame( stderr, frame_pointer );
#endif
    set_reg( frame, 0, make_cons( frame->arg[0], frame->arg[1] ) );
    set_reg( frame, 1, NIL );

    struct cons_pointer result = c_apply( frame, frame_pointer, env );

    debug_print( L"Apply returning ", DEBUG_EVAL );
    debug_dump_object( result, DEBUG_EVAL );

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
lisp_quote( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
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
lisp_set( struct stack_frame *frame, struct cons_pointer frame_pointer,
          struct cons_pointer env ) {
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
                              ( L"The first argument to `set` is not a symbol: " ),
                              make_cons( frame->arg[0], NIL ) ),
                            frame_pointer );
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
lisp_set_shriek( struct stack_frame *frame, struct cons_pointer frame_pointer,
                 struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_pointer namespace =
        nilp( frame->arg[2] ) ? oblist : frame->arg[2];

    if ( symbolp( frame->arg[0] ) ) {
        struct cons_pointer val =
            eval_form( frame, frame_pointer, frame->arg[1], env );
        deep_bind( frame->arg[0], val );
        result = val;
    } else {
        result =
            make_exception( make_cons
                            ( c_string_to_lisp_string
                              ( L"The first argument to `set!` is not a symbol: " ),
                              make_cons( frame->arg[0], NIL ) ),
                            frame_pointer );
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
lisp_cons( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
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
lisp_car( struct stack_frame *frame, struct cons_pointer frame_pointer,
          struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    if ( consp( frame->arg[0] ) ) {
        struct cons_space_object cell = pointer2cell( frame->arg[0] );
        result = cell.payload.cons.car;
    } else if ( stringp( frame->arg[0] ) ) {
        struct cons_space_object cell = pointer2cell( frame->arg[0] );
        result = make_string( cell.payload.string.character, NIL );
    } else {
        struct cons_pointer message =
            c_string_to_lisp_string( L"Attempt to take CAR of non sequence" );
        result = throw_exception( message, frame_pointer );
    }

    return result;
}

/**
 * (cdr s_expr)
 * Returns the remainder of a sequence when the head is removed. Valid for cons cells,
 * strings, and TODO read streams and other things which can be considered as sequences.
 */
struct cons_pointer
lisp_cdr( struct stack_frame *frame, struct cons_pointer frame_pointer,
          struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    if ( consp( frame->arg[0] ) ) {
        struct cons_space_object cell = pointer2cell( frame->arg[0] );
        result = cell.payload.cons.cdr;
    } else if ( stringp( frame->arg[0] ) ) {
        struct cons_space_object cell = pointer2cell( frame->arg[0] );
        result = cell.payload.string.cdr;
    } else {
        struct cons_pointer message =
            c_string_to_lisp_string( L"Attempt to take CDR of non sequence" );
        result = throw_exception( message, frame_pointer );
    }

    return result;
}

/**
 * (assoc key store)
 * Returns the value associated with key in store, or NIL if not found.
 */
struct cons_pointer
lisp_assoc( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    return c_assoc( frame->arg[0], frame->arg[1] );
}

/**
 * (eq a b)
 * Returns T if a and b are pointers to the same object, else NIL
 */
struct cons_pointer lisp_eq( struct stack_frame *frame,
                             struct cons_pointer frame_pointer,
                             struct cons_pointer env ) {
    return eq( frame->arg[0], frame->arg[1] ) ? TRUE : NIL;
}

/**
 * (eq a b)
 * Returns T if a and b are pointers to structurally identical objects, else NIL
 */
struct cons_pointer
lisp_equal( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    return equal( frame->arg[0], frame->arg[1] ) ? TRUE : NIL;
}

/**
 * (read)
 * (read read-stream)
 * Read one complete lisp form and return it. If read-stream is specified and
 * is a read stream, then read from that stream, else stdin.
 */
struct cons_pointer
lisp_read( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
#ifdef DEBUG
    debug_print( L"entering lisp_read\n", DEBUG_IO );
#endif
    FILE *input = stdin;

    if ( readp( frame->arg[0] ) ) {
        input = pointer2cell( frame->arg[0] ).payload.stream.stream;
    }

    struct cons_pointer result = read( frame, frame_pointer, input );
    debug_print( L"lisp_read returning\n", DEBUG_IO );
    debug_dump_object( result, DEBUG_IO );

    return result;
}


/**
 * reverse a sequence.
 */
struct cons_pointer c_reverse( struct cons_pointer arg ) {
    struct cons_pointer result = NIL;

    for ( struct cons_pointer p = arg; sequencep( p ); p = c_cdr( p ) ) {
        struct cons_space_object o = pointer2cell( p );
        switch ( o.tag.value ) {
            case CONSTV:
                result = make_cons( o.payload.cons.car, result );
                break;
            case STRINGTV:
                result = make_string( o.payload.string.character, result );
                break;
            case SYMBOLTV:
                result = make_symbol( o.payload.string.character, result );
                break;
        }
    }

    return result;
}


/**
 * (reverse sequence)
 * Return a sequence like this sequence but with the members in the reverse order.
 */
struct cons_pointer lisp_reverse( struct stack_frame *frame,
                                  struct cons_pointer frame_pointer,
                                  struct cons_pointer env ) {
    return c_reverse( frame->arg[0] );
}


/**
 * (print expr)
 * (print expr write-stream)
 * Print one complete lisp form and return NIL. If write-stream is specified and
 * is a write stream, then print to that stream, else stdout.
 */
struct cons_pointer
lisp_print( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    debug_print( L"Entering print\n", DEBUG_IO );
    struct cons_pointer result = NIL;
    FILE *output = stdout;

    if ( writep( frame->arg[1] ) ) {
        debug_print( L"lisp_print: setting output stream\n", DEBUG_IO );
        debug_dump_object( frame->arg[1], DEBUG_IO );
        output = pointer2cell( frame->arg[1] ).payload.stream.stream;
    }
    debug_print( L"lisp_print: about to print\n", DEBUG_IO );
    debug_dump_object( frame->arg[0], DEBUG_IO );

    result = print( output, frame->arg[0] );

    debug_print( L"lisp_print returning\n", DEBUG_IO );
    debug_dump_object( result, DEBUG_IO );

    return result;
}


/**
 * Function: Get the Lisp type of the single argument.
 * @param frame My stack frame.
 * @param env My environment (ignored).
 * @return As a Lisp string, the tag of the object which is the argument.
 */
struct cons_pointer
lisp_type( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
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
lisp_progn( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    struct cons_pointer remaining = frame->more;
    struct cons_pointer result = NIL;

    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] ); i++ ) {
        result = eval_form( frame, frame_pointer, frame->arg[i], env );
    }

    while ( consp( remaining ) ) {
        result = eval_form( frame, frame_pointer, c_car( remaining ), env );

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
lisp_cond( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    bool done = false;

    for ( int i = 0; i < args_in_frame && !done; i++ ) {
        struct cons_pointer clause_pointer = frame->arg[i];
        debug_print( L"Cond clause: ", DEBUG_EVAL );
        debug_dump_object( clause_pointer, DEBUG_EVAL );

        if ( consp( clause_pointer ) ) {
            struct cons_space_object cell = pointer2cell( clause_pointer );
            result =
                eval_form( frame, frame_pointer, c_car( clause_pointer ),
                           env );

            if ( !nilp( result ) ) {
                struct cons_pointer vals =
                    eval_forms( frame, frame_pointer, c_cdr( clause_pointer ),
                                env );

                while ( consp( vals ) ) {
                    result = c_car( vals );
                    vals = c_cdr( vals );
                }

                done = true;
            }
        } else if ( nilp( clause_pointer ) ) {
            done = true;
        } else {
            result = throw_exception( c_string_to_lisp_string
                                      ( L"Arguments to `cond` must be lists" ),
                                      frame_pointer );
        }
    }
    /* TODO: if there are more than 8 clauses we need to continue into the
     * remainder */

    return result;
}

/**
 * Throw an exception.
 * `throw_exception` is a misnomer, because it doesn't obey the calling signature of a
 * lisp function; but it is nevertheless to be preferred to make_exception. A
 * real `throw_exception`, which does, will be needed.
 * object pointing to it. Then this should become a normal lisp function
 * which expects a normally bound frame and environment, such that
 * frame->arg[0] is the message, and frame->arg[1] is the cons-space
 * pointer to the frame in which the exception occurred.
 */
struct cons_pointer
throw_exception( struct cons_pointer message,
                 struct cons_pointer frame_pointer ) {
    debug_print( L"\nERROR: ", DEBUG_EVAL );
    debug_dump_object( message, DEBUG_EVAL );
    struct cons_pointer result = NIL;

    struct cons_space_object cell = pointer2cell( message );

    if ( cell.tag.value == EXCEPTIONTV ) {
        result = message;
    } else {
        result = make_exception( message, frame_pointer );
    }

    return result;
}

/**
 * (exception <message>)
 *
 * Function. Returns an exception whose message is this `message`, and whose
 * stack frame is the parent stack frame when the function is invoked.
 * `message` does not have to be a string but should be something intelligible
 * which can be read.
 * If `message` is itself an exception, returns that instead.
 */
struct cons_pointer
lisp_exception( struct stack_frame *frame, struct cons_pointer frame_pointer,
                struct cons_pointer env ) {
    struct cons_pointer message = frame->arg[0];
    return exceptionp( message ) ? message : make_exception( message,
                                                             frame->previous );
}
