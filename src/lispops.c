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
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
 * struct cons_pointer lisp_cond( struct cons_pointer args, struct cons_pointer env,
			       struct stack_frame* frame);
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
struct cons_pointer c_car( struct cons_pointer arg) {
  struct cons_pointer result = NIL;
  
  if ( consp(arg)) {
    result = pointer2cell( arg).payload.cons.car;
  }

  return result;
}

/**
 * Implementation of cdr in C. If arg is not a cons, does not error but returns nil.
 */
struct cons_pointer c_cdr( struct cons_pointer arg) {
  struct cons_pointer result = NIL;
  
  if ( consp(arg)) {
    result = pointer2cell( arg).payload.cons.cdr;
  }

  return result;
}

/**
 * (apply fn args...)
 *
 * I'm now confused about whether at this stage I actually need an apply special form,
 * and if so how it differs from eval.
 */
struct cons_pointer lisp_apply( struct cons_pointer args, struct cons_pointer env,
			       struct stack_frame* frame) {
  struct cons_pointer result = args;
  
  if ( consp( args)) {
    lisp_eval( args, env, frame);
  }

  return result;
}

/**
 * (eval s_expr)
 *
 * Special form.
 * If s_expr is a number, NIL, or T, returns s_expr.
 * If s_expr is an unprotected string, returns the value that s_expr is bound
 * to in the evaluation environment (env).
 * If s_expr is a list, expects the car to be something that evaluates to a 
 * function or special form.
 * If a function, evaluates all the other top level elements in s_expr and
 * passes them in a stack frame as arguments to the function.
 * If a special form, passes the cdr of s_expr to the special form as argument.
 */
struct cons_pointer lisp_eval( struct cons_pointer s_expr, struct cons_pointer env,
			       struct stack_frame* previous) {
  struct cons_pointer result = s_expr;
  struct stack_frame* my_frame =
    make_stack_frame( previous, make_cons( s_expr, NIL), env);
  
  if ( consp( s_expr)) {
    /* the hard bit. Sort out what function is required and pass the
     * args to it. */
    struct cons_pointer fn_pointer = lisp_eval( c_car( s_expr), env, my_frame);
    struct cons_pointer args = c_cdr( s_expr);

    if ( specialp( fn_pointer)) {
      struct cons_space_object special = pointer2cell( fn_pointer);
      result = (*special.payload.special.executable)( args, env, previous);
    } else if ( functionp( fn_pointer)) {
      struct cons_space_object function = pointer2cell( fn_pointer);
      struct stack_frame* frame = make_stack_frame( my_frame, args, env);

      /* the trick: pass the remaining arguments and environment to
         the executable code which is the payload of the function
         object. */
      result = (*function.payload.function.executable)( frame, env);
      free_stack_frame( frame);
    } else if ( stringp( s_expr)) {
      struct cons_pointer canonical = internedp( s_expr, env);
      if ( !nilp( canonical)) {
	result = c_assoc( canonical, env);
      } else {
	struct cons_pointer message =
	  c_string_to_lisp_string( "Attempt to value of unbound name.");
	result = lisp_throw( message, my_frame);
      }
      /* the Clojure practice of having a map serve in the function
       * place of an s-expression is a good one and I should adopt it;
       * also if the object is a consp it could be interpretable
       * source code but in the long run I don't want an interpreter,
       * and if I can get away without so much the better. */
    }
  }

  free_stack_frame( my_frame);
  
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
struct cons_pointer lisp_cons(struct stack_frame* frame, struct cons_pointer env) {
  struct cons_pointer car = frame->arg[0];
  struct cons_pointer cdr = frame->arg[1];
  struct cons_pointer result;

  if ( nilp( car) && nilp( cdr)) {
    return NIL;
  } else if ( stringp( car) && stringp( cdr) &&
	      nilp( pointer2cell( car).payload.string.cdr)) {
    result = make_string( pointer2cell( car).payload.string.character, cdr);
  } else {
    result = make_cons( car, cdr);
  }

  return result;
}

/**
 * (car s_expr)
 * Returns the first item (head) of a sequence. Valid for cons cells,
 * strings, and TODO read streams and other things which can be considered as sequences.
 */
struct cons_pointer lisp_car(struct stack_frame* frame, struct cons_pointer env) {
  struct cons_pointer result = NIL;

  if ( consp( frame->arg[ 0])) {
    struct cons_space_object cell = pointer2cell( frame->arg[ 0]);
    result = cell.payload.cons.car;
  } else if ( stringp( frame->arg[ 0])) {
    struct cons_space_object cell = pointer2cell( frame->arg[ 0]);
    result = make_string( cell.payload.string.character, NIL);
  } else {
    struct cons_pointer message =
      c_string_to_lisp_string( "Attempt to take CAR/CDR of non sequence");
    result = lisp_throw( message, frame);
  }

  return result;
}


/**
 * (cdr s_expr)
 * Returns the remainder of a sequence when the head is removed. Valid for cons cells,
 * strings, and TODO read streams and other things which can be considered as sequences.
 */
struct cons_pointer lisp_cdr(struct stack_frame* frame, struct cons_pointer env) {
  struct cons_pointer result = NIL;

  if ( consp( frame->arg[ 0])) {
    struct cons_space_object cell = pointer2cell( frame->arg[ 0]);
    result = cell.payload.cons.car;
  } else if ( stringp( frame->arg[ 0])) {
    struct cons_space_object cell = pointer2cell( frame->arg[ 0]);
    result = cell.payload.string.cdr;
  } else {
    struct cons_pointer message =
      c_string_to_lisp_string( "Attempt to take CAR/CDR of non sequence");
    result = lisp_throw( message, frame);
  }

  return result;
}

/**
 * (assoc key store)
 * Returns the value associated with key in store, or NIL if not found.
 */
struct cons_pointer lisp_assoc( struct stack_frame* frame, struct cons_pointer env) {
  return c_assoc( frame->arg[ 0], frame->arg[ 1]);
}

/**
 * (eq a b)
 * Returns T if a and b are pointers to the same object, else NIL
 */
struct cons_pointer lisp_eq( struct stack_frame* frame, struct cons_pointer env) {
  return eq( frame->arg[ 0], frame->arg[ 1]) ? TRUE : NIL;
}

/**
 * (eq a b)
 * Returns T if a and b are pointers to structurally identical objects, else NIL
 */
struct cons_pointer lisp_equal( struct stack_frame* frame, struct cons_pointer env) {
  return equal( frame->arg[ 0], frame->arg[ 1]) ? TRUE : NIL;
}

/**
 * (read)
 * (read read-stream)
 * Read one complete lisp form and return it. If read-stream is specified and 
 * is a read stream, then read from that stream, else stdin.
 */
struct cons_pointer lisp_read( struct stack_frame* frame, struct cons_pointer env) {
  FILE* input = stdin;

  if ( readp( frame->arg[0])) {
    input = pointer2cell( frame->arg[0]).payload.stream.stream;
  }

  return read( input);
}

/**
 * (print expr)
 * (print expr write-stream)
 * Print one complete lisp form and return NIL. If write-stream is specified and 
 * is a write stream, then print to that stream, else stdout.
 */
struct cons_pointer lisp_print( struct stack_frame* frame, struct cons_pointer env) {
  FILE* output = stdout;

  if ( writep( frame->arg[1])) {
    output = pointer2cell( frame->arg[1]).payload.stream.stream;
  }

  print( output, frame->arg[0]);
  
  return NIL;
}


/**
 * TODO: make this do something sensible somehow.
 */
struct cons_pointer lisp_throw( struct cons_pointer message, struct stack_frame* frame) {
  fprintf( stderr, "\nERROR: ");
  print( stderr, message);
  fprintf( stderr, "\n\nAn exception was thrown and I've no idea what to do now\n");

  exit( 1);
}

