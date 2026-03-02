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

#include "arith/integer.h"
#include "arith/peano.h"
#include "debug.h"
#include "io/io.h"
#include "io/print.h"
#include "io/read.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/stack.h"
#include "memory/vectorspace.h"
#include "memory/dump.h"
#include "ops/equal.h"
#include "ops/intern.h"
#include "ops/lispops.h"

/**
 * @brief the name of the symbol to which the prompt is bound;
 * 
 * Set in init to `*prompt*`
 */
struct cons_pointer prompt_name;

/*
 * also to create in this section:
 * struct cons_pointer lisp_let( struct cons_pointer args, struct cons_pointer env,
 * struct stack_frame* frame);
 *
 * and others I haven't thought of yet.
 */

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
    debug_print_object( form, DEBUG_EVAL );
    debug_println( DEBUG_EVAL );

    struct cons_pointer result = form;
    switch ( pointer2cell( form ).tag.value ) {
            /* things which evaluate to themselves */
        case EXCEPTIONTV:
        case FREETV:           // shouldn't happen, but anyway...
        case INTEGERTV:
        case KEYTV:
        case LOOPTV:           // don't think this should happen...
        case NILTV:
        case RATIOTV:
        case REALTV:
        case READTV:
        case STRINGTV:
        case TIMETV:
        case TRUETV:
        case WRITETV:
            break;
        default:
            {
                struct cons_pointer next_pointer =
                    make_empty_frame( parent_pointer );
                // inc_ref( next_pointer );

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
            }
            break;
    }

    debug_print( L"eval_form ", DEBUG_EVAL );
    debug_print_object( form, DEBUG_EVAL );
    debug_print( L" returning: ", DEBUG_EVAL );
    debug_print_object( result, DEBUG_EVAL );
    debug_println( DEBUG_EVAL );

    return result;
}

/**
 * Evaluate all the forms in this `list` in the context of this stack `frame`
 * and this `env`, and return a list of their values. If the arg passed as
 * `list` is not in fact a list, return NIL.
 * @param frame the stack frame.
 * @param list the list of forms to be evaluated.
 * @param env the evaluation environment.
 * @return a list of the the results of evaluating the forms.
 */
struct cons_pointer eval_forms( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer list,
                                struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    while ( consp( list ) ) {
        result =
            make_cons( eval_form( frame, frame_pointer, c_car( list ), env ),
                       result );
        list = c_cdr( list );
    }

    return c_reverse( result );
}

/**
 * OK, the idea here (and I know this is less than perfect) is that the basic `try`
 * special form in PSSE takes two arguments, the first, `body`, being a list of forms,
 * and the second, `catch`, being a catch handler (which is also a list of forms).
 * Forms from `body` are evaluated in turn until one returns an exception object,
 * or until the list is exhausted. If the list was exhausted, then the value of
 * evaluating the last form in `body` is returned. If an exception was encountered,
 * then each of the forms in `catch` is evaluated and the value of the last of
 * those is returned.
 *
 * This is experimental. It almost certainly WILL change.
 */
struct cons_pointer lisp_try( struct stack_frame *frame,
                              struct cons_pointer frame_pointer,
                              struct cons_pointer env ) {
    struct cons_pointer result =
        c_progn( frame, frame_pointer, frame->arg[0], env );

    if ( exceptionp( result ) ) {
        // TODO: need to put the exception into the environment!
        result = c_progn( frame, frame_pointer, frame->arg[1],
                          make_cons( make_cons
                                     ( c_string_to_lisp_symbol
                                       ( L"*exception*" ), result ), env ) );
    }

    return result;
}


/**
 * Return the object list (root namespace).
 *
 * * (oblist)
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return the root namespace.
 */
struct cons_pointer
lisp_oblist( struct stack_frame *frame, struct cons_pointer frame_pointer,
             struct cons_pointer env ) {
    return oblist;
}

/**
 * Used to construct the body for `lambda` and `nlambda` expressions.
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
 * Construct an interpretable function. *NOTE* that if `args` is a single symbol
 * rather than a list, a varargs function will be created.
 *
 * (lambda args body)
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param frame_pointer a pointer to my stack_frame.
 * @param env the environment in which it is to be intepreted.
 * @return an interpretable function with these `args` and this `body`.
 */
struct cons_pointer
lisp_lambda( struct stack_frame *frame, struct cons_pointer frame_pointer,
             struct cons_pointer env ) {
    return make_lambda( frame->arg[0], compose_body( frame ) );
}

/**
 * Construct an interpretable special form. *NOTE* that if `args` is a single symbol
 * rather than a list, a varargs special form will be created.
 *
 * (nlambda args body)
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param frame_pointer a pointer to my stack_frame.
 * @param env the environment in which it is to be intepreted.
 * @return an interpretable special form with these `args` and this `body`.
 */
struct cons_pointer
lisp_nlambda( struct stack_frame *frame, struct cons_pointer frame_pointer,
              struct cons_pointer env ) {
    return make_nlambda( frame->arg[0], compose_body( frame ) );
}


/**
 * Evaluate a lambda or nlambda expression.
 */
struct cons_pointer
eval_lambda( struct cons_space_object *cell, struct stack_frame *frame,
             struct cons_pointer frame_pointer, struct cons_pointer env ) {
    struct cons_pointer result = NIL;
#ifdef DEBUG
    debug_print( L"eval_lambda called\n", DEBUG_LAMBDA );
    debug_println( DEBUG_LAMBDA );
#endif

    struct cons_pointer new_env = env;
    struct cons_pointer names = cell->payload.lambda.args;
    struct cons_pointer body = cell->payload.lambda.body;

    if ( consp( names ) ) {
        /* if `names` is a list, bind successive items from that list
         * to values of arguments */
        for ( int i = 0; i < frame->args && consp( names ); i++ ) {
            struct cons_pointer name = c_car( names );
            struct cons_pointer val = frame->arg[i];

            new_env = set( name, val, new_env );
            debug_print_binding( name, val, false, DEBUG_BIND );

            names = c_cdr( names );
        }
//        inc_ref( new_env );

        /* \todo if there's more than `args_in_frame` arguments, bind those too. */
    } else if ( symbolp( names ) ) {
        /* if `names` is a symbol, rather than a list of symbols,
         * then bind a list of the values of args to that symbol. */
        /* \todo eval all the things in frame->more */
        struct cons_pointer vals =
            eval_forms( frame, frame_pointer, frame->more, env );

        for ( int i = args_in_frame - 1; i >= 0; i-- ) {
            struct cons_pointer val =
                eval_form( frame, frame_pointer, frame->arg[i], env );

            if ( nilp( val ) && nilp( vals ) ) {  /* nothing */
            } else {
                vals = make_cons( val, vals );
            }
        }

        new_env = set( names, vals, new_env );
//        inc_ref( new_env );
    }

    while ( !nilp( body ) ) {
        struct cons_pointer sexpr = c_car( body );
        body = c_cdr( body );

        debug_print( L"In lambda: evaluating ", DEBUG_LAMBDA );
        debug_print_object( sexpr, DEBUG_LAMBDA );
        // debug_print( L"\t env is: ", DEBUG_LAMBDA );
        // debug_print_object( new_env, DEBUG_LAMBDA );
        debug_println( DEBUG_LAMBDA );

        /* if a result is not the terminal result in the lambda, it's a
         * side effect, and needs to be GCed */
        if ( !nilp( result ) ) {
            dec_ref( result );
        }

        result = eval_form( frame, frame_pointer, sexpr, new_env );

        if ( exceptionp( result ) ) {
            break;
        }
    }

    // dec_ref( new_env );

    debug_print( L"eval_lambda returning: \n", DEBUG_LAMBDA );
    debug_print_object( result, DEBUG_LAMBDA );
    debug_println( DEBUG_LAMBDA );

    return result;
}

/**
 * if `r` is an exception, and it doesn't have a location, fix up its location from
 * the name associated with this fn_pointer, if any. 
 */
struct cons_pointer maybe_fixup_exception_location( struct cons_pointer r,
                                                    struct cons_pointer
                                                    fn_pointer ) {
    struct cons_pointer result = r;

    if ( exceptionp( result )
         && ( functionp( fn_pointer ) || specialp( fn_pointer ) ) ) {
        struct cons_space_object *fn_cell = &pointer2cell( fn_pointer );

        struct cons_pointer payload =
            pointer2cell( result ).payload.exception.payload;
        /* TODO: should name_key also be a privileged keyword? */
        struct cons_pointer name_key = c_string_to_lisp_keyword( L"name" );

        switch ( get_tag_value( payload ) ) {
            case NILTV:
            case CONSTV:
            case HASHTV:
                {
                    if ( nilp( c_assoc( privileged_keyword_location,
                                        payload ) ) ) {
                        pointer2cell( result ).payload.exception.payload =
                            set( privileged_keyword_location,
                                 c_assoc( name_key,
                                          fn_cell->payload.function.meta ),
                                 payload );
                    }
                }
                break;
            default:
                pointer2cell( result ).payload.exception.payload =
                    make_cons( make_cons( privileged_keyword_location,
                                          c_assoc( name_key,
                                                   fn_cell->payload.
                                                   function.meta ) ),
                               make_cons( make_cons
                                          ( privileged_keyword_payload,
                                            payload ), NIL ) );
        }

        dec_ref( name_key );
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
    debug_print( L"Entering c_apply\n", DEBUG_EVAL );
    struct cons_pointer result = NIL;

    struct cons_pointer fn_pointer =
        eval_form( frame, frame_pointer, c_car( frame->arg[0] ), env );

    if ( exceptionp( fn_pointer ) ) {
        result = fn_pointer;
    } else {
        struct cons_space_object *fn_cell = &pointer2cell( fn_pointer );
        struct cons_pointer args = c_cdr( frame->arg[0] );

        switch ( get_tag_value( fn_pointer ) ) {
            case EXCEPTIONTV:
                /* just pass exceptions straight back */
                result = fn_pointer;
                break;

            case FUNCTIONTV:
                {
                    struct cons_pointer exep = NIL;
                    struct cons_pointer next_pointer =
                        make_stack_frame( frame_pointer, args, env );
//                    inc_ref( next_pointer );
                    if ( exceptionp( next_pointer ) ) {
                        result = next_pointer;
                    } else {
                        struct stack_frame *next =
                            get_stack_frame( next_pointer );

                        result = maybe_fixup_exception_location( ( *
                                                                   ( fn_cell->
                                                                     payload.
                                                                     function.
                                                                     executable ) )
                                                                 ( next,
                                                                   next_pointer,
                                                                   env ),
                                                                 fn_pointer );
                        dec_ref( next_pointer );
                    }
                }
                break;

            case KEYTV:
                result = c_assoc( fn_pointer,
                                  eval_form( frame,
                                             frame_pointer,
                                             c_car( c_cdr( frame->arg[0] ) ),
                                             env ) );
                break;

            case LAMBDATV:
                {
                    struct cons_pointer exep = NIL;
                    struct cons_pointer next_pointer =
                        make_stack_frame( frame_pointer, args, env );
//                    inc_ref( next_pointer );
                    if ( exceptionp( next_pointer ) ) {
                        result = next_pointer;
                    } else {
                        struct stack_frame *next =
                            get_stack_frame( next_pointer );
                        result =
                            eval_lambda( fn_cell, next, next_pointer, env );
                        if ( !exceptionp( result ) ) {
                            dec_ref( next_pointer );
                        }
                    }
                }
                break;

            case HASHTV:
                /* \todo: if arg[0] is a CONS, treat it as a path */
                result = c_assoc( eval_form( frame,
                                             frame_pointer,
                                             c_car( c_cdr
                                                    ( frame->arg
                                                      [0] ) ), env ),
                                  fn_pointer );
                break;

            case NLAMBDATV:
                {
                    struct cons_pointer next_pointer =
                        make_special_frame( frame_pointer, args, env );
//                    inc_ref( next_pointer );
                    if ( exceptionp( next_pointer ) ) {
                        result = next_pointer;
                    } else {
                        struct stack_frame *next =
                            get_stack_frame( next_pointer );
                        result =
                            eval_lambda( fn_cell, next, next_pointer, env );
                        dec_ref( next_pointer );
                    }
                }
                break;

            case SPECIALTV:
                {
                    struct cons_pointer next_pointer =
                        make_special_frame( frame_pointer, args, env );
                    // inc_ref( next_pointer );
                    if ( exceptionp( next_pointer ) ) {
                        result = next_pointer;
                    } else {
                        result = maybe_fixup_exception_location( ( *
                                                                   ( fn_cell->
                                                                     payload.
                                                                     special.
                                                                     executable ) )
                                                                 ( get_stack_frame( next_pointer ), next_pointer, env ), fn_pointer );
                        debug_print( L"Special form returning: ", DEBUG_EVAL );
                        debug_print_object( result, DEBUG_EVAL );
                        debug_println( DEBUG_EVAL );
                        dec_ref( next_pointer );
                    }
                }
                break;

            default:
                {
                    int bs = sizeof( wchar_t ) * 1024;
                    wchar_t *buffer = malloc( bs );
                    memset( buffer, '\0', bs );
                    swprintf( buffer, bs,
                              L"Unexpected cell with tag %d (%4.4s) in function position",
                              fn_cell->tag.value, &( fn_cell->tag.bytes[0] ) );
                    struct cons_pointer message =
                        c_string_to_lisp_string( buffer );
                    free( buffer );
                    result =
                        throw_exception( c_string_to_lisp_symbol( L"apply" ),
                                         message, frame_pointer );
                }
        }

    }

    debug_print( L"c_apply: returning: ", DEBUG_EVAL );
    debug_print_object( result, DEBUG_EVAL );
    debug_println( DEBUG_EVAL );

    return result;
}

/**
 * Function; evaluate the expression which is the first argument in the frame;
 * further arguments are ignored.
 *
 * * (eval expression)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment.
 * @return
 * * If `expression` is a number, string, `nil`, or `t`, returns `expression`.
 * * If `expression` is a symbol, returns the value that expression is bound
 * to in the evaluation environment (`env`).
 * * If `expression` is a list, expects the car to be something that evaluates to a
 * function or special form:
 *   * If a function, evaluates all the other top level elements in `expression` and
 * passes them in a stack frame as arguments to the function;
 *   * If a special form, passes the cdr of expression to the special form as argument.
 * @exception if `expression` is a symbol which is not bound in `env`.
 */
struct cons_pointer
lisp_eval( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
    debug_print( L"Eval: ", DEBUG_EVAL );
    debug_dump_object( frame_pointer, DEBUG_EVAL );

    struct cons_pointer result = frame->arg[0];
    struct cons_space_object *cell = &pointer2cell( frame->arg[0] );

    switch ( cell->tag.value ) {
        case CONSTV:
            result = c_apply( frame, frame_pointer, env );
            break;

        case SYMBOLTV:
            {
                struct cons_pointer canonical = interned( frame->arg[0], env );
                if ( nilp( canonical ) ) {
                    struct cons_pointer message =
                        make_cons( c_string_to_lisp_string
                                   ( L"Attempt to take value of unbound symbol." ),
                                   frame->arg[0] );
                    result =
                        throw_exception( c_string_to_lisp_symbol( L"eval" ),
                                         message, frame_pointer );
                } else {
                    result = c_assoc( canonical, env );
                    inc_ref( result );
                }
            }
            break;
            /*
             * \todo
             * the Clojure practice of having a map serve in the function place of
             * an s-expression is a good one and I should adopt it; 
             * H'mmm... this is working, but it isn't here. Where is it?
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
 * Function; apply the function which is the result of evaluating the
 * first argument to the list of values which is the result of evaluating
 * the second argument
 *
 * * (apply fn args)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment.
 * @return the result of applying `fn` to `args`.
 */
struct cons_pointer
lisp_apply( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    debug_print( L"Apply: ", DEBUG_EVAL );
    debug_dump_object( frame_pointer, DEBUG_EVAL );

    set_reg( frame, 0, make_cons( frame->arg[0], frame->arg[1] ) );
    set_reg( frame, 1, NIL );

    struct cons_pointer result = c_apply( frame, frame_pointer, env );

    debug_print( L"Apply returning ", DEBUG_EVAL );
    debug_dump_object( result, DEBUG_EVAL );

    return result;
}


/**
 * Special form;
 * returns its argument (strictly first argument - only one is expected but
 * this isn't at this stage checked) unevaluated.
 *
 * * (quote a)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return `a`, unevaluated,
 */
struct cons_pointer
lisp_quote( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    return frame->arg[0];
}


/**
 * Function;
 * binds the value of `name` in the `namespace` to value of `value`, altering
 * the namespace in so doing. Retuns `value`.
 * `namespace` defaults to the oblist.
 * \todo doesn't actually work yet for namespaces which are not the oblist.
 *
 * * (set name value)
 * * (set name value namespace)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return `value`
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
            throw_exception( c_string_to_lisp_symbol( L"set" ),
                             make_cons
                             ( c_string_to_lisp_string
                               ( L"The first argument to `set` is not a symbol: " ),
                               make_cons( frame->arg[0], NIL ) ),
                             frame_pointer );
    }

    return result;
}


/**
 * Special form;
 * binds `symbol` in the `namespace` to value of `value`, altering
 * the namespace in so doing, and returns value. `namespace` defaults to
 * the value of `oblist`.
 * \todo doesn't actually work yet for namespaces which are not the oblist.
 *
 * * (set! symbol value)
 * * (set! symbol value namespace)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return `value`
 */
struct cons_pointer
lisp_set_shriek( struct stack_frame *frame, struct cons_pointer frame_pointer,
                 struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_pointer namespace = frame->arg[2];

    if ( symbolp( frame->arg[0] ) ) {
        struct cons_pointer val =
            eval_form( frame, frame_pointer, frame->arg[1], env );
        deep_bind( frame->arg[0], val );
        result = val;
    } else {
        result =
            throw_exception( c_string_to_lisp_symbol( L"set!" ),
                             make_cons
                             ( c_string_to_lisp_string
                               ( L"The first argument to `set!` is not a symbol: " ),
                               make_cons( frame->arg[0], NIL ) ),
                             frame_pointer );
    }

    return result;
}

/**
 * @return true if `arg` represents an end of string, else false.
 * \todo candidate for moving to a memory/string.c file
 */
bool end_of_stringp( struct cons_pointer arg ) {
    return nilp( arg ) ||
        ( stringp( arg ) &&
          pointer2cell( arg ).payload.string.character == ( wint_t ) '\0' );
}

/**
 * Function;
 * returns a cell constructed from a and b. If a is of type string but its
 * cdr is nill, and b is of type string, then returns a new string cell;
 * otherwise returns a new cons cell.
 * 
 * Thus: `(cons "a" "bcd") -> "abcd"`, but `(cons "ab" "cd") -> ("ab" . "cd")`
 *
 * * (cons a b)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return a new cons cell whose `car` is `a` and whose `cdr` is `b`.
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
                end_of_stringp( c_cdr( car ) ) ) {
        result =
            make_string( pointer2cell( car ).payload.string.character, cdr );
    } else {
        result = make_cons( car, cdr );
    }

    return result;
}

/**
 * Function;
 * returns the first item (head) of a sequence. Valid for cons cells,
 * strings, read streams and TODO other things which can be considered as sequences.
 *
 * * (car expression)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return the first item (head) of `expression`.
 * @exception if `expression` is not a sequence.
 */
struct cons_pointer
lisp_car( struct stack_frame *frame, struct cons_pointer frame_pointer,
          struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_space_object *cell = &pointer2cell( frame->arg[0] );

    switch ( cell->tag.value ) {
        case CONSTV:
            result = cell->payload.cons.car;
            break;
        case NILTV:
            break;
        case READTV:
            result =
                make_string( url_fgetwc( cell->payload.stream.stream ), NIL );
            break;
        case STRINGTV:
            result = make_string( cell->payload.string.character, NIL );
            break;
        default:
            result =
                throw_exception( c_string_to_lisp_symbol( L"car" ),
                                 c_string_to_lisp_string
                                 ( L"Attempt to take CAR of non sequence" ),
                                 frame_pointer );
    }

    return result;
}

/**
 * Function;
 * returns the remainder of a sequence when the head is removed. Valid for cons cells,
 * strings, read streams and TODO other things which can be considered as sequences.
 * *NOTE* that if the argument is an input stream, the first character is removed AND
 * DISCARDED.
 *
 * * (cdr expression)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return the remainder of `expression` when the head is removed.
 * @exception if `expression` is not a sequence.
 */
struct cons_pointer
lisp_cdr( struct stack_frame *frame, struct cons_pointer frame_pointer,
          struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_space_object *cell = &pointer2cell( frame->arg[0] );

    switch ( cell->tag.value ) {
        case CONSTV:
            result = cell->payload.cons.cdr;
            break;
        case NILTV:
            break;
        case READTV:
            url_fgetwc( cell->payload.stream.stream );
            result = frame->arg[0];
            break;
        case STRINGTV:
            result = cell->payload.string.cdr;
            break;
        default:
            result =
                throw_exception( c_string_to_lisp_symbol( L"cdr" ),
                                 c_string_to_lisp_string
                                 ( L"Attempt to take CDR of non sequence" ),
                                 frame_pointer );
    }

    return result;
}

/**
 * Function: return, as an integer, the length of the sequence indicated by
 * the first argument, or zero if it is not a sequence.
 *
 * * (length any)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return the length of `any`, if it is a sequence, or zero otherwise.
 */
struct cons_pointer lisp_length( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 struct cons_pointer env ) {
    return make_integer( c_length( frame->arg[0] ), NIL );
}

/**
 * Function; look up the value of a `key` in a `store`.
 *
 * * (assoc key store)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return the value associated with `key` in `store`, or `nil` if not found.
 */
struct cons_pointer
lisp_assoc( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    return c_assoc( frame->arg[0],
                    nilp( frame->arg[1] ) ? oblist : frame->arg[1] );
}

/**
 * @brief `(interned? key store)`: Return `t` if the symbol or keyword `key` is bound in this `store`, else `nil`.
 * 
 * @param frame 
 * @param frame_pointer 
 * @param env 
 * @return struct cons_pointer 
 */
struct cons_pointer
lisp_internedp( struct stack_frame *frame, struct cons_pointer frame_pointer,
                struct cons_pointer env ) {
    struct cons_pointer result = internedp( frame->arg[0],
                                            nilp( frame->arg[1] ) ? oblist :
                                            frame->arg[1] );

    if ( exceptionp( result ) ) {
        struct cons_pointer old = result;
        struct cons_space_object *cell = &( pointer2cell( result ) );
        result =
            throw_exception( c_string_to_lisp_symbol( L"interned?" ),
                             cell->payload.exception.payload, frame_pointer );
        dec_ref( old );
    }

    return result;
}

struct cons_pointer c_keys( struct cons_pointer store ) {
    struct cons_pointer result = NIL;

    if ( hashmapp( store ) ) {
        result = hashmap_keys( store );
    } else if ( consp( store ) ) {
        for ( struct cons_pointer c = store; !nilp( c ); c = c_cdr( c ) ) {
            result = make_cons( c_car( c ), result );
        }
    }

    return result;
}

struct cons_pointer lisp_keys( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               struct cons_pointer env ) {
    return c_keys( frame->arg[0] );
}

/**
 * Function; are these two objects the same object? Shallow, cheap equality.
 *
 * * (eq a b)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return `t` if `a` and `b` are pointers to the same object, else `nil`;
 */
struct cons_pointer lisp_eq( struct stack_frame *frame,
                             struct cons_pointer frame_pointer,
                             struct cons_pointer env ) {
    struct cons_pointer result = TRUE;

    if ( frame->args > 1 ) {
        for ( int b = 1; ( truep( result ) ) && ( b < frame->args ); b++ ) {
            result = eq( frame->arg[0], fetch_arg( frame, b ) ) ? TRUE : NIL;
        }
    }

    return result;
}

/**
 * Function; are these two arguments identical? Deep, expensive equality.
 *
 * * (equal a b)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return `t` if `a` and `b` are recursively identical, else `nil`.
 */
struct cons_pointer
lisp_equal( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    struct cons_pointer result = TRUE;

    if ( frame->args > 1 ) {
        for ( int b = 1; ( truep( result ) ) && ( b < frame->args ); b++ ) {
            result =
                equal( frame->arg[0], fetch_arg( frame, b ) ) ? TRUE : NIL;
        }
    }

    return result;
}

long int c_count( struct cons_pointer p ) {
    struct cons_space_object *cell = &pointer2cell( p );
    int result = 0;

    switch ( cell->tag.value ) {
        case CONSTV:
        case STRINGTV:
            /* I think doctrine is that you cannot treat symbols or keywords as
             * sequences, although internally, of course, they are. Integers are
             * also internally sequences, but also should not be treated as such.
             */
            for ( p; !nilp( p ); p = c_cdr( p ) ) {
                result++;
            }
    }

    return result;
}

/**
 * Function: return the number of top level forms in the object which is
 * the first (and only) argument, if it is a sequence (which for current
 * purposes means a list or a string)
 *
 * * (count l)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return the number of top level forms in a list, or characters in a
 *         string, else 0.
 */
struct cons_pointer
lisp_count( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    return acquire_integer( c_count( frame->arg[0] ), NIL );
}

/**
 * Function; read one complete lisp form and return it. If read-stream is specified and
 * is a read stream, then read from that stream, else the stream which is the value of
 * `*in*` in the environment.
 *
 * * (read)
 * * (read read-stream)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment.
 * @return the expression read.
 */
struct cons_pointer
lisp_read( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
#ifdef DEBUG
    debug_print( L"entering lisp_read\n", DEBUG_IO );
#endif
    URL_FILE *input;

    struct cons_pointer in_stream = readp( frame->arg[0] ) ?
        frame->arg[0] : get_default_stream( true, env );

    if ( readp( in_stream ) ) {
        debug_print( L"lisp_read: setting input stream\n",
                     DEBUG_IO | DEBUG_REPL );
        debug_dump_object( in_stream, DEBUG_IO );
        input = pointer2cell( in_stream ).payload.stream.stream;
        inc_ref( in_stream );
    } else {
        /* should not happen, but has done. */
        debug_print( L"WARNING: invalid input stream; defaulting!\n",
                     DEBUG_IO | DEBUG_REPL );
        input = file_to_url_file( stdin );
    }

    struct cons_pointer result = read( frame, frame_pointer, env, input );
    debug_print( L"lisp_read returning\n", DEBUG_IO );
    debug_dump_object( result, DEBUG_IO );

    if ( readp( in_stream ) ) {
        dec_ref( in_stream );
    } else {
        free( input );
    }


    return result;
}


/**
 * reverse a sequence (if it is a sequence); else return it unchanged.
 */
struct cons_pointer c_reverse( struct cons_pointer arg ) {
    struct cons_pointer result = NIL;

    if ( sequencep( arg ) ) {
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
                    result =
                        make_symbol_or_key( o.payload.string.character, result,
                                            SYMBOLTV );
                    break;
            }
        }
    } else {
        result = arg;
    }

    return result;
}


/**
 * Function; reverse the order of members in s sequence.
 *
 * * (reverse sequence)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return a sequence like this `sequence` but with the members in the reverse order.
 */
struct cons_pointer lisp_reverse( struct stack_frame *frame,
                                  struct cons_pointer frame_pointer,
                                  struct cons_pointer env ) {
    return c_reverse( frame->arg[0] );
}

/**
 * Function: dump/inspect one complete lisp expression and return NIL. If
 * write-stream is specified and is a write stream, then print to that stream,
 * else  the stream which is the value of
 * `*out*` in the environment.
 *
 * * (inspect expr)
 * * (inspect expr write-stream)
 *
 * @param frame my stack_frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (from which the stream may be extracted).
 * @return NIL.
 */
struct cons_pointer lisp_inspect( struct stack_frame *frame,
                                  struct cons_pointer frame_pointer,
                                  struct cons_pointer env ) {
    debug_print( L"Entering lisp_inspect\n", DEBUG_IO );
    struct cons_pointer result = NIL;
    struct cons_pointer out_stream = writep( frame->arg[1] )
        ? frame->arg[1]
        : get_default_stream( false, env );
    URL_FILE *output;

    if ( writep( out_stream ) ) {
        debug_print( L"lisp_inspect: setting output stream\n", DEBUG_IO );
        debug_dump_object( out_stream, DEBUG_IO );
        output = pointer2cell( out_stream ).payload.stream.stream;
    } else {
        output = file_to_url_file( stderr );
    }

    dump_object( output, frame->arg[0] );

    debug_print( L"Leaving lisp_inspect", DEBUG_IO );

    return result;
}


/**
 * Function: get the Lisp type of the single argument.
 *
 * * (type expression)
 *
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env my environment (ignored).
 * @return As a Lisp string, the tag of `expression`.
 */
struct cons_pointer
lisp_type( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
    return c_type( frame->arg[0] );
}

/**
 * Evaluate each of these expressions in this `env`ironment over this `frame`,
 * returning only the value of the last.
 */
struct cons_pointer
c_progn( struct stack_frame *frame, struct cons_pointer frame_pointer,
         struct cons_pointer expressions, struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    while ( consp( expressions ) ) {
        struct cons_pointer r = result;
        inc_ref( r );
        result = eval_form( frame, frame_pointer, c_car( expressions ), env );
        dec_ref( r );

        expressions = exceptionp( result ) ? NIL : c_cdr( expressions );
    }

    return result;
}


/**
 * Special form; evaluate the expressions which are listed in my arguments
 * sequentially and return the value of the last. This function is called 'do'
 * in some dialects of Lisp.
 *
 * * (progn expressions...)
 *
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env the environment in which expressions are evaluated.
 * @return the value of the last `expression` of the sequence which is my single
 * argument.
 */
struct cons_pointer
lisp_progn( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env ) {
    struct cons_pointer result = NIL;

    for ( int i = 0; i < args_in_frame && !nilp( frame->arg[i] ); i++ ) {
        struct cons_pointer r = result;
        inc_ref( r );

        result = eval_form( frame, frame_pointer, frame->arg[i], env );

        dec_ref( r );
    }

    if ( consp( frame->more ) ) {
        result = c_progn( frame, frame_pointer, frame->more, env );
    }

    return result;
}

/**
 * @brief evaluate a single cond clause; if the test part succeeds return a 
 * pair whose car is TRUE and whose cdr is the value of the action part 
 */
struct cons_pointer eval_cond_clause( struct cons_pointer clause,
                                      struct stack_frame *frame,
                                      struct cons_pointer frame_pointer,
                                      struct cons_pointer env ) {
    struct cons_pointer result = NIL;

#ifdef DEBUG
    debug_print( L"\n\tCond clause: ", DEBUG_EVAL );
    debug_print_object( clause, DEBUG_EVAL );
    debug_println( DEBUG_EVAL );
#endif

    if ( consp( clause ) ) {
        struct cons_pointer val =
            eval_form( frame, frame_pointer, c_car( clause ),
                       env );

        if ( !nilp( val ) ) {
            result =
                make_cons( TRUE,
                           c_progn( frame, frame_pointer, c_cdr( clause ),
                                    env ) );

#ifdef DEBUG
            debug_print( L"\n\t\tclause succeeded; returning: ", DEBUG_EVAL );
            debug_print_object( result, DEBUG_EVAL );
            debug_println( DEBUG_EVAL );
        } else {
            debug_print( L"\n\t\tclause failed.\n", DEBUG_EVAL );
#endif
        }
    } else {
        result = throw_exception( c_string_to_lisp_symbol( L"cond" ),
                                  c_string_to_lisp_string
                                  ( L"Arguments to `cond` must be lists" ),
                                  frame_pointer );
    }

    return result;
}

/**
 * Special form: conditional. Each `clause` is expected to be a list; if the first
 * item in such a list evaluates to non-NIL, the remaining items in that list
 * are evaluated in turn and the value of the last returned. If no arg `clause`
 * has a first element which evaluates to non NIL, then NIL is returned.
 *
 * * (cond clauses...)
 *
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env the environment in which arguments will be evaluated.
 * @return the value of the last expression of the first successful `clause`.
 */
struct cons_pointer
lisp_cond( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    bool done = false;

    for ( int i = 0; ( i < frame->args ) && !done; i++ ) {
        struct cons_pointer clause_pointer = fetch_arg( frame, i );

        result = eval_cond_clause( clause_pointer, frame, frame_pointer, env );

        if ( !nilp( result ) && truep( c_car( result ) ) ) {
            result = c_cdr( result );
            done = true;
            break;
        }
    }
#ifdef DEBUG
    debug_print( L"\tCond returning: ", DEBUG_EVAL );
    debug_print_object( result, DEBUG_EVAL );
    debug_println( DEBUG_EVAL );
#endif

    return result;
}

/**
 * Throw an exception with a cause.
 * `throw_exception` is a misnomer, because it doesn't obey the calling signature of a
 * lisp function; but it is nevertheless to be preferred to make_exception. A
 * real `throw_exception`, which does, will be needed.
 * object pointing to it. Then this should become a normal lisp function
 * which expects a normally bound frame and environment, such that
 * frame->arg[0] is the payload, frame->arg[1] is the cause, and frame->arg[2] is the cons-space
 * pointer to the frame in which the exception occurred.
 */
struct cons_pointer throw_exception_with_cause( struct cons_pointer location,
                 struct cons_pointer message,
                 struct cons_pointer cause,
                 struct cons_pointer frame_pointer ) {
    struct cons_pointer result = NIL;

#ifdef DEBUG
    debug_print( L"\nERROR: `", 511 );
    debug_print_object( message, 511 );
    debug_print( L"` at `", 511 );
    debug_print_object( location, 511 );
    debug_print( L"`\n", 511 );
    if (!nilp( cause)) {
        debug_print( L"\tCaused by: ", 511)
        ;
        debug_print_object( cause, 511);
        debug_print( L"`\n", 511 );
    }
#endif
    struct cons_space_object *cell = &pointer2cell( message );

    if ( cell->tag.value == EXCEPTIONTV ) {
        result = message;
    } else {
        result =
            make_exception( make_cons
                            ( make_cons( privileged_keyword_location,
                                         location ),
                              make_cons( make_cons
                                         ( privileged_keyword_payload,
                                           message ), 
                                        (nilp( cause) ? NIL :
                                    make_cons( make_cons( privileged_keyword_cause,
                                    cause), NIL)) ) ), frame_pointer );
    }

    return result;

}

/**
 * Throw an exception.
 * `throw_exception` is a misnomer, because it doesn't obey the calling signature of a
 * lisp function; but it is nevertheless to be preferred to make_exception. A
 * real `throw_exception`, which does, will be needed.
 * object pointing to it. Then this should become a normal lisp function
 * which expects a normally bound frame and environment, such that
 * frame->arg[0] is the payload, frame->arg[1] is the cause, and frame->arg[2] is the cons-space
 * pointer to the frame in which the exception occurred.
 */
struct cons_pointer
throw_exception( struct cons_pointer location,
                 struct cons_pointer payload,
                 struct cons_pointer frame_pointer ) {
    return throw_exception_with_cause( location, payload, NIL, frame_pointer);
}

/**
 * Function; create an exception. Exceptions are special in as much as if an
 * exception is created in the binding of the arguments of any function, the
 * function will return the exception rather than whatever else it would
 * normally return. A function which detects a problem it cannot resolve
 * *should* return an exception.
 *
 * * (exception message location)
 *
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env the environment in which arguments will be evaluated.
 * @return areturns an exception whose message is this `message`, and whose
 * stack frame is the parent stack frame when the function is invoked.
 * `message` does not have to be a string but should be something intelligible
 * which can be read.
 * If `message` is itself an exception, returns that instead.
 */
struct cons_pointer
lisp_exception( struct stack_frame *frame, struct cons_pointer frame_pointer,
                struct cons_pointer env ) {
    struct cons_pointer message = frame->arg[0];

    return exceptionp( message ) ? message : throw_exception_with_cause( message,
                                                              frame->arg[1],
                                                              frame->arg[2],
                                                              frame->previous );
}

/**
 * Function: the read/eval/print loop.
 *
 * * (repl)
 * * (repl prompt)
 * * (repl prompt input_stream output_stream)
 *
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env the environment in which epressions will be evaluated.
 * @return the value of the last expression read.
 */
struct cons_pointer lisp_repl( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               struct cons_pointer env ) {
    struct cons_pointer expr = NIL;

#ifdef DEBUG
    debug_print( L"Entering new inner REPL\n\tenv is `", DEBUG_REPL );
    debug_print_object( env, DEBUG_REPL );
    debug_print( L"`\n", DEBUG_REPL );
#endif

    struct cons_pointer input = get_default_stream( true, env );
    struct cons_pointer output = get_default_stream( false, env );
    struct cons_pointer old_oblist = oblist;
    struct cons_pointer new_env = env;

    if ( truep( frame->arg[0] ) ) {
        new_env = set( prompt_name, frame->arg[0], new_env );
    }
    if ( readp( frame->arg[1] ) ) {
        new_env =
            set( c_string_to_lisp_symbol( L"*in*" ), frame->arg[1], new_env );
        input = frame->arg[1];
    }
    if ( writep( frame->arg[2] ) ) {
        new_env =
            set( c_string_to_lisp_symbol( L"*out*" ), frame->arg[2], new_env );
        output = frame->arg[2];
    }

    inc_ref( input );
    inc_ref( output );
    inc_ref( prompt_name );

    /* output should NEVER BE nil; but during development it has happened.
     * To allow debugging under such circumstances, we need an emergency 
     * default. */
    URL_FILE *os =
        !writep( output ) ? file_to_url_file( stdout ) :
        pointer2cell( output ).payload.stream.stream;
    if ( !writep( output ) ) {
        debug_print( L"WARNING: invalid output; defaulting!\n",
                     DEBUG_IO | DEBUG_REPL );
    }

    /* \todo this is subtly wrong. If we were evaluating
     *   (print (eval (read)))
     * then the stack frame for read would have the stack frame for
     * eval as parent, and it in turn would have the stack frame for
     * print as parent.
     */
    while ( readp( input ) && writep( output )
            && !url_feof( pointer2cell( input ).payload.stream.stream ) ) {
        /* OK, here's a really subtle problem: because lists are immutable, anything
         * bound in the oblist subsequent to this function being invoked isn't in the
         * environment. So, for example, changes to *prompt* or *log* made in the oblist
         * are not visible. So copy changes made in the oblist into the enviroment.
         * \todo the whole process of resolving symbol values needs to be revisited
         * when we get onto namespaces. */
        /* OK, there's something even more subtle here if the root namespace is a map.
         * H'mmmm... 
         * I think that now the oblist is a hashmap masquerading as a namespace,
         * we should no longer have to do this. TODO: test, and if so, delete this
         * statement. */
        if ( !eq( oblist, old_oblist ) ) {
            struct cons_pointer cursor = oblist;

            while ( !nilp( cursor ) && !eq( cursor, old_oblist ) ) {
                struct cons_pointer old_new_env = new_env;
                debug_print
                    ( L"lisp_repl: copying new oblist binding into REPL environment:\n",
                      DEBUG_REPL );
                debug_print_object( c_car( cursor ), DEBUG_REPL );
                debug_println( DEBUG_REPL );

                new_env = make_cons( c_car( cursor ), new_env );
                inc_ref( new_env );
                dec_ref( old_new_env );
                cursor = c_cdr( cursor );
            }
            old_oblist = oblist;
        }

        println( os );

        struct cons_pointer prompt = c_assoc( prompt_name, new_env );
        if ( !nilp( prompt ) ) {
            print( os, prompt );
        }

        expr = lisp_read( get_stack_frame( frame_pointer ), frame_pointer,
                          new_env );

        if ( exceptionp( expr )
             && url_feof( pointer2cell( input ).payload.stream.stream ) ) {
            /* suppress printing end of stream exception */
            dec_ref( expr );
            break;
        }

        println( os );

        print( os, eval_form( frame, frame_pointer, expr, new_env ) );

        dec_ref( expr );
    }

    if ( nilp( output ) ) {
        free( os );
    }
    dec_ref( input );
    dec_ref( output );
    dec_ref( prompt_name );
    dec_ref( new_env );

    debug_printf( DEBUG_REPL, L"Leaving inner repl\n" );

    return expr;
}

/**
 * Function. return the source code of the object which is its first argument,
 * if it is an executable and has source code.
 *
 * * (source object)
 *
 * @param frame my stack frame.
 * @param frame_pointer a pointer to my stack_frame.
 * @param env the environment (ignored).
 * @return the source of the `object` indicated, if it is a function, a lambda,
 * an nlambda, or a spcial form; else `nil`.
 */
struct cons_pointer lisp_source( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    struct cons_space_object *cell = &pointer2cell( frame->arg[0] );
    struct cons_pointer source_key = c_string_to_lisp_keyword( L"source" );
    switch ( cell->tag.value ) {
        case FUNCTIONTV:
            result = c_assoc( source_key, cell->payload.function.meta );
            break;
        case SPECIALTV:
            result = c_assoc( source_key, cell->payload.special.meta );
            break;
        case LAMBDATV:
            result = make_cons( c_string_to_lisp_symbol( L"lambda" ),
                                make_cons( cell->payload.lambda.args,
                                           cell->payload.lambda.body ) );
            break;
        case NLAMBDATV:
            result = make_cons( c_string_to_lisp_symbol( L"nlambda" ),
                                make_cons( cell->payload.lambda.args,
                                           cell->payload.lambda.body ) );
            break;
    }
    // \todo suffers from premature GC, and I can't see why!
    inc_ref( result );

    return result;
}

/**
 * A version of append which can conveniently be called from C.
 */
struct cons_pointer c_append( struct cons_pointer l1, struct cons_pointer l2 ) {
    switch ( pointer2cell( l1 ).tag.value ) {
        case CONSTV:
            if ( pointer2cell( l1 ).tag.value == pointer2cell( l2 ).tag.value ) {
                if ( nilp( c_cdr( l1 ) ) ) {
                    return make_cons( c_car( l1 ), l2 );
                } else {
                    return make_cons( c_car( l1 ),
                                      c_append( c_cdr( l1 ), l2 ) );
                }
            } else {
                throw_exception( c_string_to_lisp_symbol( L"append" ),
                                 c_string_to_lisp_string
                                 ( L"Can't append: not same type" ), NIL );
            }
            break;
        case KEYTV:
        case STRINGTV:
        case SYMBOLTV:
            if ( pointer2cell( l1 ).tag.value == pointer2cell( l2 ).tag.value ) {
                if ( nilp( c_cdr( l1 ) ) ) {
                    return
                        make_string_like_thing( ( pointer2cell( l1 ).payload.
                                                  string.character ), l2,
                                                pointer2cell( l1 ).tag.value );
                } else {
                    return
                        make_string_like_thing( ( pointer2cell( l1 ).payload.
                                                  string.character ),
                                                c_append( c_cdr( l1 ), l2 ),
                                                pointer2cell( l1 ).tag.value );
                }
            } else {
                throw_exception( c_string_to_lisp_symbol( L"append" ),
                                 c_string_to_lisp_string
                                 ( L"Can't append: not same type" ), NIL );
            }
            break;
        default:
            throw_exception( c_string_to_lisp_symbol( L"append" ),
                             c_string_to_lisp_string
                             ( L"Can't append: not a sequence" ), NIL );
            break;
    }
}

/**
 * should really be overwritten with a version in Lisp, since this is much easier to write in Lisp
 */
struct cons_pointer lisp_append( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 struct cons_pointer env ) {
    struct cons_pointer result = fetch_arg( frame, ( frame->args - 1 ) );

    for ( int a = frame->args - 2; a >= 0; a-- ) {
        result = c_append( fetch_arg( frame, a ), result );
    }

    return result;
}

struct cons_pointer lisp_mapcar( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 struct cons_pointer env ) {
    struct cons_pointer result = NIL;
    debug_print( L"Mapcar: ", DEBUG_EVAL );
    debug_dump_object( frame_pointer, DEBUG_EVAL );
    int i = 0;

    for ( struct cons_pointer c = frame->arg[1]; truep( c ); c = c_cdr( c ) ) {
        struct cons_pointer expr =
            make_cons( frame->arg[0], make_cons( c_car( c ), NIL ) );
        inc_ref( expr );

        debug_printf( DEBUG_EVAL, L"Mapcar %d, evaluating ", i );
        debug_print_object( expr, DEBUG_EVAL );
        debug_println( DEBUG_EVAL );

        struct cons_pointer r = eval_form( frame, frame_pointer, expr, env );

        if ( exceptionp( r ) ) {
            result = r;
            inc_ref( expr );    // to protect exception from the later dec_ref
            break;
        } else {
            result = make_cons( r, result );
        }
        debug_printf( DEBUG_EVAL, L"Mapcar %d, result is ", i++ );
        debug_print_object( result, DEBUG_EVAL );
        debug_println( DEBUG_EVAL );

        dec_ref( expr );
    }

    result = consp( result ) ? c_reverse( result ) : result;

    debug_print( L"Mapcar returning: ", DEBUG_EVAL );
    debug_print_object( result, DEBUG_EVAL );
    debug_println( DEBUG_EVAL );

    return result;
}

/**
 * @brief construct and return a list of arbitrarily many arguments.
 * 
 * @param frame The stack frame.
 * @param frame_pointer A pointer to the stack frame.
 * @param env The evaluation environment.
 * @return struct cons_pointer a pointer to the result
 */
struct cons_pointer lisp_list( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               struct cons_pointer env ) {
    struct cons_pointer result = frame->more;

    for ( int a = nilp( result ) ? frame->args - 1 : args_in_frame - 1;
          a >= 0; a-- ) {
        result = make_cons( fetch_arg( frame, a ), result );
    }

    return result;
}



/**
 * Special form: evaluate a series of forms in an environment in which
 * these bindings are bound.
 * This is `let*` in Common Lisp parlance; `let` in Clojure parlance.
 */
struct cons_pointer lisp_let( struct stack_frame *frame,
                              struct cons_pointer frame_pointer,
                              struct cons_pointer env ) {
    struct cons_pointer bindings = env;
    struct cons_pointer result = NIL;

    for ( struct cons_pointer cursor = frame->arg[0];
          truep( cursor ); cursor = c_cdr( cursor ) ) {
        struct cons_pointer pair = c_car( cursor );
        struct cons_pointer symbol = c_car( pair );

        if ( symbolp( symbol ) ) {
            struct cons_pointer val =
                eval_form( frame, frame_pointer, c_cdr( pair ),
                           bindings );

            debug_print_binding( symbol, val, false, DEBUG_BIND );

            bindings = make_cons( make_cons( symbol, val ), bindings );
        } else {
            result =
                throw_exception( c_string_to_lisp_symbol( L"let" ),
                                 c_string_to_lisp_string
                                 ( L"Let: cannot bind, not a symbol" ),
                                 frame_pointer );
            break;
        }
    }

    debug_print( L"\nlet: bindings complete.\n", DEBUG_BIND );

    /* i.e., no exception yet */
    for ( int form = 1; !exceptionp( result ) && form < frame->args; form++ ) {
        result =
            eval_form( frame, frame_pointer, fetch_arg( frame, form ),
                       bindings );
    }

    /* release the local bindings as they go out of scope! **BUT** 
     * bindings were consed onto the front of env, so caution... */
    // for (struct cons_pointer cursor = bindings; !eq( cursor, env); cursor = c_cdr(cursor)) {
    //     dec_ref( cursor);
    // }

    return result;

}

/**
 * @brief Boolean `and` of arbitrarily many arguments.
 * 
 * @param frame The stack frame.
 * @param frame_pointer A pointer to the stack frame.
 * @param env The evaluation environment.
 * @return struct cons_pointer a pointer to the result
 */
struct cons_pointer lisp_and( struct stack_frame *frame,
                              struct cons_pointer frame_pointer,
                              struct cons_pointer env ) {
    bool accumulator = true;
    struct cons_pointer result = frame->more;

    for ( int a = 0; accumulator == true && a < frame->args; a++ ) {
        accumulator = truthy( fetch_arg( frame, a ) );
    }
#
    return accumulator ? TRUE : NIL;
}

/**
 * @brief Boolean `or` of arbitrarily many arguments.
 * 
 * @param frame The stack frame.
 * @param frame_pointer A pointer to the stack frame.
 * @param env The evaluation environment.
 * @return struct cons_pointer a pointer to the result
 */
struct cons_pointer lisp_or( struct stack_frame *frame,
                             struct cons_pointer frame_pointer,
                             struct cons_pointer env ) {
    bool accumulator = false;
    struct cons_pointer result = frame->more;

    for ( int a = 0; accumulator == false && a < frame->args; a++ ) {
        accumulator = truthy( fetch_arg( frame, a ) );
    }

    return accumulator ? TRUE : NIL;
}

/**
 * @brief Logical inverese: if the first argument is `nil`, return `t`, else `nil`.
 * 
 * @param frame The stack frame.
 * @param frame_pointer A pointer to the stack frame.
 * @param env The evaluation environment.
 * @return struct cons_pointer `t` if the first argument is `nil`, else `nil`.
 */
struct cons_pointer lisp_not( struct stack_frame *frame,
                              struct cons_pointer frame_pointer,
                              struct cons_pointer env ) {
    return nilp( frame->arg[0] ) ? TRUE : NIL;
}
