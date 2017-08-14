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

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <wchar.h>

#include "version.h"
#include "conspage.h"
#include "consspaceobject.h"
#include "intern.h"
#include "lispops.h"
#include "repl.h"

void bind_function( char* name, struct cons_pointer (*executable)
		    (struct stack_frame*, struct cons_pointer)) {
  deep_bind( intern( c_string_to_lisp_symbol( name), oblist ),
	     make_function( NIL, executable));
}

void bind_special( char* name, struct cons_pointer (*executable)
		   (struct cons_pointer s_expr, struct cons_pointer env,
		    struct stack_frame* frame)) {
  deep_bind( intern( c_string_to_lisp_symbol( name), oblist ),
	     make_special( NIL, executable));
}

int main (int argc, char *argv[]) {
    /* attempt to set wide character acceptance on all streams */
    fwide(stdin, 1);
    fwide(stdout, 1);
    fwide(stderr, 1);
    int option;
    bool dump_at_end = false;
    bool show_prompt = false;
    
    while ((option = getopt (argc, argv, "pd")) != -1)
    {
      switch (option)
      {
        case 'd':
          dump_at_end = true;
          break;
        case 'p':
          show_prompt = true;
          break;
        default:
          fprintf( stderr, "Unexpected option %c\n", option);
          break;
      }
    }

    if (show_prompt) {
      fprintf( stdout, "Post scarcity software environment version %s\n\n", VERSION);
    }
    
    initialise_cons_pages();

    /* privileged variables (keywords) */
    deep_bind( intern( c_string_to_lisp_string( "nil"), oblist), NIL);
    deep_bind( intern( c_string_to_lisp_string( "t"), oblist), TRUE);

    /* primitive function operations */
    bind_function( "assoc", &lisp_assoc);
    bind_function( "car", &lisp_car);
    bind_function( "cdr", &lisp_cdr);
    bind_function( "cons", &lisp_cons);
    bind_function( "eq", &lisp_eq);
    bind_function( "equal", &lisp_equal);
    bind_function( "read", &lisp_read);
    bind_function( "print", &lisp_print);

    /* primitive special forms */
    bind_special( "apply", &lisp_apply);
    bind_special( "eval", &lisp_eval);
    bind_special( "quote", &lisp_quote);

    repl(stdin, stdout, stderr, show_prompt);
    // print( stdout, lisp_eval( input, oblist, NULL));

    if ( dump_at_end) {
        dump_pages(stderr);
    }

    return(0);
}
