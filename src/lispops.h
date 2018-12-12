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

/*
 * utilities
 */

/**
 * Get the Lisp type of the single argument.
 * @param pointer a pointer to the object whose type is requested.
 * @return As a Lisp string, the tag of the object which is at that pointer.
 */
struct cons_pointer c_type( struct cons_pointer pointer );

/*
 * special forms 
 */
struct cons_pointer lisp_eval( struct stack_frame *frame,
                               struct cons_pointer env );
struct cons_pointer lisp_apply( struct stack_frame *frame,
                                struct cons_pointer env );
                                /**
 * The Lisp interpreter.
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param lexpr the lambda expression to be interpreted;
 * @param env the environment in which it is to be intepreted.
 */
struct cons_pointer lisp_lambda( struct stack_frame *frame,
                                 struct cons_pointer env );

struct cons_pointer lisp_quote( struct stack_frame *frame,
                                struct cons_pointer env );

/*
 * functions 
 */
struct cons_pointer lisp_cons( struct stack_frame *frame,
                               struct cons_pointer env );
struct cons_pointer lisp_car( struct stack_frame *frame,
                              struct cons_pointer env );
struct cons_pointer lisp_cdr( struct stack_frame *frame,
                              struct cons_pointer env );
struct cons_pointer lisp_assoc( struct stack_frame *frame,
                                struct cons_pointer env );
struct cons_pointer lisp_eq( struct stack_frame *frame,
                             struct cons_pointer env );
struct cons_pointer lisp_equal( struct stack_frame *frame,
                                struct cons_pointer env );
struct cons_pointer lisp_read( struct stack_frame *frame,
                               struct cons_pointer env );
struct cons_pointer lisp_print( struct stack_frame *frame,
                                struct cons_pointer env );
/**
 * Function: Get the Lisp type of the single argument.
 * @param frame My stack frame.
 * @param env My environment (ignored).
 * @return As a Lisp string, the tag of the object which is the argument.
 */
struct cons_pointer
lisp_type( struct stack_frame *frame, struct cons_pointer env );


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
lisp_progn( struct stack_frame *frame, struct cons_pointer env );

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
lisp_cond( struct stack_frame *frame, struct cons_pointer env );

/*
 * neither, at this stage, really 
 */
struct cons_pointer lisp_throw( struct cons_pointer message,
                                struct stack_frame *frame );
