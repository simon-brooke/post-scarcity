/**
 * init.c
 *
 * Start up and initialise the environement - just enough to get working 
 * and (ultimately) hand off to the executive.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdio.h>

#include "version.h"
#include "conspage.h"
#include "consspaceobject.h"
#include "intern.h"
#include "lispops.h"
#include "print.h"
#include "read.h"

void bind_function( char* name, struct cons_pointer (*executable)
		    (struct stack_frame*, struct cons_pointer)) {
  deep_bind( intern( c_string_to_lisp_string( name), oblist ),
	     make_function( NIL, executable));
}

void bind_special( char* name, struct cons_pointer (*executable)
		   (struct cons_pointer s_expr, struct cons_pointer env,
		    struct stack_frame* frame)) {
  deep_bind( intern( c_string_to_lisp_string( name), oblist ),
	     make_special( NIL, executable));
}

int main (int argc, char *argv[]) {
  fprintf( stderr, "Post scarcity software environment version %s\n", VERSION);
  initialise_cons_pages();

  bind_function( "assoc", &lisp_assoc);
  bind_function( "car", &lisp_car);
  bind_function( "cdr", &lisp_cdr);
  bind_function( "cons", &lisp_cons);
  bind_function( "eq", &lisp_eq);
  bind_function( "equal", &lisp_equal);
  bind_function( "read", &lisp_read);
  bind_function( "print", &lisp_print);

  bind_special( "apply", &lisp_apply);
  bind_special( "eval", &lisp_eval);

  fprintf( stderr, "\n:: ");
  struct cons_pointer input = read( stdin);
  fprintf( stderr, "\n{%d,%d}=> ", input.page, input.offset);
  print( stdout, input);

  dump_pages(stderr);
  
  return(0);
}
