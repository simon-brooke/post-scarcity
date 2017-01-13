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
#include "print.h"
#include "read.h"
#include "lispops.h"

int main (int argc, char *argv[]) {
  fprintf( stderr, "Post scarcity software environment version %s\n", VERSION);
  initialise_cons_pages();

  fprintf( stderr, "\n:: ");
  struct cons_pointer input = read( stdin);
  fprintf( stderr, "\n{%d,%d}=> ", input.page, input.offset);
  print( stdout, input);

  dump_pages(stderr);
  
  return(0);
}
