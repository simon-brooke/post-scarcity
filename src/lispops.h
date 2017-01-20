/**
 * lispops.h
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
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

/* special forms */
struct cons_pointer lisp_eval( struct cons_pointer args, struct cons_pointer env,
			       struct stack_frame* frame);
struct cons_pointer lisp_apply( struct cons_pointer args, struct cons_pointer env,
				struct stack_frame* frame);
struct cons_pointer lisp_quote( struct cons_pointer args, struct cons_pointer env,
				struct stack_frame* frame);

/* functions */
struct cons_pointer lisp_cons( struct stack_frame* frame, struct cons_pointer env);
struct cons_pointer lisp_car( struct stack_frame* frame, struct cons_pointer env);
struct cons_pointer lisp_cdr( struct stack_frame* frame, struct cons_pointer env);
struct cons_pointer lisp_assoc( struct stack_frame* frame, struct cons_pointer env);
struct cons_pointer lisp_eq( struct stack_frame* frame, struct cons_pointer env);
struct cons_pointer lisp_equal( struct stack_frame* frame, struct cons_pointer env);
struct cons_pointer lisp_read( struct stack_frame* frame, struct cons_pointer env);
struct cons_pointer lisp_print( struct stack_frame* frame, struct cons_pointer env);

/* neither, at this stage, really */
struct cons_pointer lisp_throw( struct cons_pointer message, struct stack_frame* frame);
