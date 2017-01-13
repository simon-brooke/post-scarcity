/**
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
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "consspaceobject.h"
#include "conspage.h"
#include "integer.h"
#include "read.h"

struct cons_pointer lisp_eval( struct cons_pointer args, struct cons_pointer env);
struct cons_pointer lisp_cons( struct cons_pointer args, struct cons_pointer env);
struct cons_pointer lisp_car( struct cons_pointer args, struct cons_pointer env);
struct cons_pointer lisp_cdr( struct cons_pointer args, struct cons_pointer env);
struct cons_pointer lisp_apply( struct cons_pointer args, struct cons_pointer env);
struct cons_pointer lisp_throw( struct cons_pointer args, struct cons_pointer env);

/**
 * in the long run this becomes redundant because it's just (map eval
 * args), but it helps bootstrapping.
 */
struct cons_pointer i_eval_args( struct cons_pointer args, struct cons_pointer tail, struct cons_pointer env) {
  struct cons_pointer result = NIL;
  
  if ( ! nilp( args)) {
    result = make_cons( lisp_eval( lisp_car( args, env), env),
			i_eval_args( lisp_cdr( args, env), tail, env));
  }

  return result;
}

/**
 * (apply fn args...)
 * I'm assuming that the function should be protected from evaluation
 * in apply because its name has already been resolved to the function
 * object in read.
 */
struct cons_pointer lisp_apply( struct cons_pointer args, struct cons_pointer env) {
  struct cons_pointer result = args;
  
  if ( consp( args)) {
    lisp_eval( make_cons( lisp_car( args, env), i_eval_args( lisp_cdr( args, env), NIL, env)), env);
  }

  return result;
}

/**
 * (eval args)
 */
struct cons_pointer lisp_eval( struct cons_pointer args, struct cons_pointer env) {
  struct cons_pointer result = args;
  
  if ( consp( args)) {
    /* the hard bit. Sort out what function is required and pass the
     * args to it. */
    struct cons_pointer fn_pointer = lisp_car( args, env);
    args = lisp_cdr( args, env);

    if ( functionp( fn_pointer)) {
      struct cons_space_object function = pointer2cell( fn_pointer);

      /* the trick: pass the remaining arguments and environment to
         the executable code which is the payload of the function
         object. */
      result = (*function.payload.function.executable)( args, env);
    } else {
      /* the Clojure practice of having a map serve in the function
       * place of an s-expression is a good one and I should adopt it;
       * also if the object is a consp it could be interpretable
       * source code but in the long run I don't want an interpreter,
       * and if I can get away without so much the better. */
      result = lisp_throw( args, env);
    }
  }

  return result;
}

/**
 * Apply cons to this argsument list. Effectively, create a cons cell comprising
 * (car args) (cadr args).
 */ 
struct cons_pointer lisp_cons( struct cons_pointer args, struct cons_pointer env) {
  struct cons_pointer result = NIL;

  if ( consp( args)) {
    struct cons_space_object cell = pointer2cell( args);
    struct cons_pointer a = cell.payload.cons.car;
    struct cons_pointer d = pointer2cell( cell.payload.cons.cdr).payload.cons.car;
    result = make_cons( a, d);
  } else {
    lisp_throw( args, env);
  }

  return result;
}

/**
 * Apply car to this argsument list. Effectively, (car (car args))
 */
struct cons_pointer lisp_car( struct cons_pointer args, struct cons_pointer env) {
  struct cons_pointer result = NIL;

  if ( consp( args)) {
    struct cons_space_object cell = pointer2cell( args);
    result =  pointer2cell( cell.payload.cons.car).payload.cons.car;
  } else {
    lisp_throw( args, env);
  }

  return result;
}


/**
 * Apply cdr to this argsument list. Effectively, (cdr (car args))
 */
struct cons_pointer lisp_cdr( struct cons_pointer args, struct cons_pointer env) {
  struct cons_pointer result = NIL;

  if ( consp( args)) {
    struct cons_space_object cell = pointer2cell( args);
    result =  pointer2cell( cell.payload.cons.cdr).payload.cons.car;
  } else {
    lisp_throw( args, env);
  }

  return result;
}


/**
 * TODO: make this do something sensible somehow.
 */
struct cons_pointer lisp_throw( struct cons_pointer args, struct cons_pointer env) {
  fprintf( stderr, "An exception was thrown and I've no idea what to do now\n");

  return NIL;
}

