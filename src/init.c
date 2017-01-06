/**
 * init.c
 *
 * Start up and initialise the environement - just enough to get working and (ultimately)
 * hand off to the executive.
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

int main (int argc, char *argv[]) {
  printf( "Post scarcity software environment version %s\n", VERSION);
  conspagesinit();

  printf( "Ready\n>>");

  struct cons_pointer input = read( stdin);
  incref( input);
  printf( "\n");
  print( stdout, input);

  dumppages(stdout);
  // printf( "Tag2uint(\"FREE\") = %d\n", tag2uint("FREE"));
  
  return(0);
}
