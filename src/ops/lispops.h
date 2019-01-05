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

/**
 * Implementation of car in C. If arg is not a cons, does not error but returns nil.
 */
struct cons_pointer c_car( struct cons_pointer arg );

/**
 * Implementation of cdr in C. If arg is not a cons, does not error but returns nil.
 */
struct cons_pointer c_cdr( struct cons_pointer arg );

struct cons_pointer c_reverse( struct cons_pointer arg );

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
                               struct cons_pointer env );

/**
 * eval all the forms in this `list` in the context of this stack `frame`
 * and this `env`, and return a list of their values. If the arg passed as
 * `list` is not in fact a list, return nil.
 */
struct cons_pointer eval_forms( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer list,
                                struct cons_pointer env );


/*
 * special forms
 */
struct cons_pointer lisp_eval( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               struct cons_pointer env );
struct cons_pointer lisp_apply( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer env );

struct cons_pointer
lisp_oblist( struct stack_frame *frame, struct cons_pointer frame_pointer,
             struct cons_pointer env );

struct cons_pointer
lisp_set( struct stack_frame *frame, struct cons_pointer frame_pointer,
          struct cons_pointer env );

struct cons_pointer
lisp_set_shriek( struct stack_frame *frame, struct cons_pointer frame_pointer,
                 struct cons_pointer env );

/**
 * Construct an interpretable function.
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param lexpr the lambda expression to be interpreted;
 * @param env the environment in which it is to be intepreted.
 */
struct cons_pointer lisp_lambda( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 struct cons_pointer env );

/**
 * Construct an interpretable special form.
 *
 * @param frame the stack frame in which the expression is to be interpreted;
 * @param env the environment in which it is to be intepreted.
 */
struct cons_pointer
lisp_nlambda( struct stack_frame *frame, struct cons_pointer frame_pointer,
              struct cons_pointer env );

struct cons_pointer lisp_quote( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer env );

/*
 * functions
 */
struct cons_pointer lisp_cons( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               struct cons_pointer env );
struct cons_pointer lisp_car( struct stack_frame *frame,
                              struct cons_pointer frame_pointer,
                              struct cons_pointer env );
struct cons_pointer lisp_cdr( struct stack_frame *frame,
                              struct cons_pointer frame_pointer,
                              struct cons_pointer env );
struct cons_pointer lisp_assoc( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer env );
struct cons_pointer lisp_eq( struct stack_frame *frame,
                             struct cons_pointer frame_pointer,
                             struct cons_pointer env );
struct cons_pointer lisp_equal( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer env );
struct cons_pointer lisp_print( struct stack_frame *frame,
                                struct cons_pointer frame_pointer,
                                struct cons_pointer env );
struct cons_pointer lisp_read( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               struct cons_pointer env );
struct cons_pointer lisp_repl( struct stack_frame *frame,
                               struct cons_pointer frame_pointer,
                               struct cons_pointer env );
struct cons_pointer lisp_reverse( struct stack_frame *frame,
                                  struct cons_pointer frame_pointer,
                                  struct cons_pointer env );

/**
 * Function: Get the Lisp type of the single argument.
 * @param frame My stack frame.
 * @param env My environment (ignored).
 * @return As a Lisp string, the tag of the object which is the argument.
 */
struct cons_pointer
lisp_type( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env );


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
lisp_progn( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env );

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
           struct cons_pointer env );

/**
 * Throw an exception.
 * `throw_exception` is a misnomer, because it doesn't obey the calling signature of a
 * lisp function; but it is nevertheless to be preferred to make_exception. A
 * real `throw_exception`, which does, will be needed.
 */
struct cons_pointer throw_exception( struct cons_pointer message,
                                     struct cons_pointer frame_pointer );

struct cons_pointer
lisp_exception( struct stack_frame *frame, struct cons_pointer frame_pointer,
                struct cons_pointer env );

struct cons_pointer lisp_source( struct stack_frame *frame,
                                 struct cons_pointer frame_pointer,
                                 struct cons_pointer env );

struct cons_pointer lisp_inspect( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env );
